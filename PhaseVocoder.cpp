#include "PhaseVocoder.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctime>
#include <iostream>
#include <bitset>
#include <chrono>
#include <math.h>
#include <fstream>
#include <iomanip>

PhaseVocoder::PhaseVocoder() :
        m_window_size(WINDOW_SIZE), m_hop_size(HOP_SIZE),
        m_forward_fft(FFT_ORDER), m_reverse_fft(FFT_ORDER)
{
    m_pitch_ratio = 2.0f;
    for (uint32_t i = 0; i < SLIDER_COUNT; i++)
    {
        if (i == 0)
        {
            m_sliders[i].slider_min = 0.0f;
            m_sliders[i].slider_max = 110.0f;
        } 
        else
        {
            m_sliders[i].slider_min = (float)(55 << i);
            m_sliders[i].slider_max = (float)(55 << (i+1));
        }
        m_sliders[i].slider_median = (m_sliders[i].slider_max + m_sliders[i].slider_min) / 2;
    }
    uint32_t bin_pos;
    for (uint32_t i = 0; i < FREQUENCY_MAX; i++)
    {
        bin_pos = (uint32_t)((float)i * (float)FFT_SIZE / (SAMPLE_RATE));
        if (bin_pos > FFT_SIZE / 2)
        {
            bin_pos = FFT_SIZE - bin_pos;
        }
        m_freq_to_bin[i] = bin_pos;
    }
    for (uint32_t i = 0; i < FFT_SIZE; i++)
    {
        m_bin_ratio[i] = i;
        uint32_t bin_frequency;
        if (i <= FFT_SIZE / 2)
            bin_frequency = (uint32_t)((float)i * SAMPLE_RATE / (float)FFT_SIZE);
        else
            bin_frequency = (uint32_t)((float)(FFT_SIZE - i) * SAMPLE_RATE / (float)FFT_SIZE);
        m_bin_to_freq[i] = bin_frequency;
        m_bin_to_slider[i] = GetSlider(bin_frequency);
    }
    GenerateWindowFunction();
    for (uint32_t i = 0; i < THREAD_COUNT; i++)
    {
        for (uint32_t j = 0; j < FFT_SIZE * 2; j++)
        {
            m_input_buffer[i][j] = 0.0f;
            m_complex_intermed_fw[i][j] = dsp::Complex<float>(0.0f, 0.0f);
            m_complex_intermed_rv[i][j] = dsp::Complex<float>(0.0f, 0.0f);
            m_complex_out[i][j] = dsp::Complex<float>(0.0f, 0.0f);
            if (j < FFT_SIZE)
            {
                m_psi[i][j] = 0.0f;
                m_last_phase[i][j] = 0.0f;
            }
        }
    }
}

inline uint32_t PhaseVocoder::GetSlider(uint32_t frequency_input)
{
    uint32_t base_frequency = 55;
    if (frequency_input < 110)
    {
        return 0;
    }
    for (uint32_t i = 0; i < SLIDER_COUNT; i++)
    {
        if ((base_frequency << i) <= frequency_input && (base_frequency << (i + 1)) > frequency_input)
        {
            return i;
        }
    }

    // Exceeds last slider in frequency, return last slider
    return (SLIDER_COUNT - 1);
}

void PhaseVocoder::Finish()
{
}


void PhaseVocoder::GenerateWindowFunction()
{
    for (uint32_t curr_pos = 0; curr_pos < m_window_size; curr_pos++)
    {
        m_window_function[curr_pos] = (float)0.5*((float)1 - (float)cos((2 * M_PI * curr_pos) / ((float)m_window_size - 1 + 1e-20)));
    }
}

PhaseVocoder::~PhaseVocoder()
{
}

void PhaseVocoder::PhaseLock()
{

}

void PhaseVocoder::Process(dsp::Complex<float> * time_domain, dsp::Complex<float> * frequency_domain, dsp::Complex<float> * time_domain_output, uint32_t fft_size, ProcessType type, uint32_t channel)
{
    // Consider allocating F domain buffer here?
    switch (type)
    {
    case ProcessType::PitchShift:
        {
            dsp::Complex<float> time_domain_intermediate[FFT_SIZE];
            PerformFFT(time_domain, frequency_domain, false);
            PitchShift(frequency_domain, fft_size, channel);
            PerformFFT(frequency_domain, time_domain_intermediate, true);

            float output_length = floor(fft_size / m_pitch_ratio);
            float x, dx;
            uint32_t ix;
            dsp::Complex<float> *resampled_output = new dsp::Complex<float>[(uint32_t)output_length];
            for (uint32_t i = 0; i < (uint32_t)output_length; i++)
            {
                x = i * fft_size / output_length;
                ix = (uint32_t)floor(x);
                dx = x - (float)ix;
                resampled_output[i].real(time_domain_intermediate[ix].real() * (1.0f - dx) + time_domain_intermediate[(ix + 1) % fft_size].real() * dx);
            }

            uint32_t output_buffer_index = 0;
            for (uint32_t i = 0; i < (uint32_t)output_length; i++)
            {
                if (i > m_window_size)
                    break;
                else
                    time_domain_output[output_buffer_index].real(time_domain_output[output_buffer_index].real() + resampled_output[i].real() * m_window_function[i]);
                if (++output_buffer_index >= fft_size)
                    output_buffer_index = 0;
            }
            delete[] resampled_output;
        }
        break;
    case ProcessType::Robotization:
        PerformFFT(time_domain, frequency_domain, false);
        Robotization(frequency_domain, fft_size);
        PerformFFT(frequency_domain, time_domain_output, true);
        break;
    case ProcessType::Whisperization:
        PerformFFT(time_domain, frequency_domain, false);
        Whisperization(frequency_domain, fft_size);
        PerformFFT(frequency_domain, time_domain_output, true);
        break;
    case ProcessType::Phaser:
        PerformFFT(time_domain, frequency_domain, false);
        Phaser(frequency_domain, fft_size);
        PerformFFT(frequency_domain, time_domain_output, true);
        break;
    case ProcessType::NoneDebug:
        PerformFFT(time_domain, frequency_domain, false);
        PerformFFT(frequency_domain, time_domain_output, true);
        break;
    case ProcessType::BinShift:
        PerformFFT(time_domain, frequency_domain, false);
        BinShift(frequency_domain, fft_size);
        PerformFFT(frequency_domain, time_domain_output, true);
    default:
        break;
    }
}
 
inline float get_random_phase()
{
    return 2.0f * (float)M_PI * ((float)rand() / (float)RAND_MAX);
}

void PhaseVocoder::Whisperization(dsp::Complex<float> * fft_data, uint32_t fft_size)
{
    float cos_phase, sin_phase;
    float amplitude = abs(fft_data[0]);
    float phase = get_random_phase();

    fft_data[0].real(amplitude * cosf(phase));
    fft_data[0].imag(amplitude * sinf(phase));

    for (uint32_t bin = 1; bin < (fft_size / 2 - 1); bin++)
    {
        amplitude = abs(fft_data[bin]);

        phase = get_random_phase();
        cos_phase = cosf(phase);
        sin_phase = sinf(phase);
        fft_data[bin].real(amplitude * cos_phase);
        fft_data[bin].imag(amplitude * sin_phase);
        fft_data[fft_size - bin].real(amplitude * cos_phase);
        fft_data[fft_size - bin].imag(amplitude * -sin_phase);
    }
    uint32_t bin = fft_size / 2;
    amplitude = abs(fft_data[bin]);
    phase = get_random_phase();

    fft_data[bin].real(amplitude * cosf(phase));
    fft_data[bin].imag(amplitude * sinf(phase));
}

void PhaseVocoder::change_effect(const float new_value)
{
    m_effect = new_value;
}

void PhaseVocoder::change_type(const ProcessType new_type)
{
    m_type = new_type;
}

void PhaseVocoder::change_slider_val(const float new_value, const uint32_t slider_idx)
{
    m_sliders[slider_idx].mean = new_value;
    m_sliders[slider_idx].output_bin = (uint32_t)floorf(new_value * (float)FFT_SIZE / SAMPLE_RATE);
}

void PhaseVocoder::change_range_val(const float new_value, const uint32_t slider_idx)
{
    m_sliders[slider_idx].range = new_value;
}

void PhaseVocoder::change_slider_en(const bool new_value, const uint32_t slider_idx)
{
    m_sliders[slider_idx].enable = new_value;
}

void PhaseVocoder::change_range_en(const bool new_value, const uint32_t slider_idx)
{
    m_sliders[slider_idx].range_enable = new_value;
}

void PhaseVocoder::Robotization(dsp::Complex<float> * fft_data, uint32_t fft_size)
{
    uint32_t bin = 0;
    float magnitude = abs(fft_data[bin]);
    fft_data[bin].real(magnitude);
    fft_data[bin].imag(0.0f);
    for (bin = 1; bin < (fft_size / 2 - 1); bin++)
    {
        magnitude = abs(fft_data[bin]);
        fft_data[bin].real(magnitude);
        fft_data[bin].imag(0.0f);
        fft_data[FFT_SIZE - bin].real(magnitude);
        fft_data[FFT_SIZE - bin].imag(0.0f);
    }
    bin = fft_size / 2;
    magnitude = abs(fft_data[bin]);
    fft_data[bin].real(magnitude);
    fft_data[bin].imag(0.0f);
}

inline float princarg(float input)
{
    float a = input / (2.0f * (float)M_PI);
    return input - (floor(a) * (2.0f * (float)M_PI));
}

void PhaseVocoder::PitchShift(dsp::Complex<float>* fft_data, uint32_t fft_size, uint32_t channel)
{
    dsp::Complex<float> temp_data[FFT_SIZE];
    float magnitude, phase, bin_frequency, delta_phi;
    for (uint32_t i = 0; i < fft_size; i++)
    {
        // Convert the bin into magnitude-phase representation
        magnitude = abs(fft_data[i]);
        phase = arg(fft_data[i]);
        bin_frequency = 2.0f * (float)M_PI * (float)i / fft_size;
        delta_phi = (bin_frequency * m_hop_size) + princarg(phase - m_last_phase[channel][i] - (bin_frequency * m_hop_size));
        m_last_phase[channel][i] = phase;
        m_psi[channel][i] = princarg(m_psi[channel][i] + delta_phi * m_hop_size);

        // Convert back to real-imaginary form
        temp_data[i].real(magnitude * cosf(m_psi[channel][i]));
        temp_data[i].imag(magnitude * sinf(m_psi[channel][i]));
    }
}

void PhaseVocoder::BinShift(dsp::Complex<float>* fft_data, uint32_t fft_size)
{
    dsp::Complex<float> temp[FFT_SIZE];
    uint32_t frequency;
    float slider_pos;
    float slider_scaled;
    uint32_t final_frequency;
    uint32_t final_bin;
    slider_info * _slider;
    for (uint32_t bin = 0; bin < fft_size; bin++)
    {
        _slider = &m_sliders[m_bin_to_slider[bin]];
        if (_slider->enable)
        {
            if (_slider->range_enable)
            {
                frequency = m_bin_to_freq[bin];
                
                slider_pos = ((float)frequency - _slider->slider_min) / (_slider->slider_max - _slider->slider_min);
                slider_scaled = (slider_pos * _slider->range) - (_slider->range / 2);
                final_frequency = m_bin_to_freq[_slider->output_bin] + (int)slider_scaled;
                if (final_frequency >= FREQUENCY_MAX)
                    final_frequency = FREQUENCY_MAX - 1;
                final_bin = m_freq_to_bin[final_frequency];
                temp[final_bin] += fft_data[bin];
            }
            else
            {
                temp[_slider->output_bin] += fft_data[bin];
            }
        }
        else
        {
            temp[bin] += fft_data[bin];
        }
    }
    for (uint32_t bin = 0; bin < fft_size; bin++)
    {
        fft_data[bin] = temp[bin];
    }
}

void PhaseVocoder::Phaser(dsp::Complex<float>* fft_data, uint32_t fft_size)
{
    uint32_t bin = 0;
    float magnitude = abs(fft_data[bin]);
    float _cos = cosf((float)M_PI * 2 * m_effect);
    float _sin = sinf((float)M_PI * 2 * m_effect);
    fft_data[bin].real(_cos * magnitude);
    fft_data[bin].imag(_sin * magnitude);
    for (bin = 1; bin < (fft_size / 2 - 1); bin++)
    {
        magnitude = abs(fft_data[bin]);
        fft_data[bin].real(_cos * magnitude);
        fft_data[bin].imag(_sin * magnitude);
        fft_data[FFT_SIZE - bin].real(_cos * magnitude);
        fft_data[FFT_SIZE - bin].imag(- _sin * magnitude);
    }
    bin = fft_size / 2;
    magnitude = abs(fft_data[bin]);
    fft_data[bin].real(_cos * magnitude);
    fft_data[bin].imag(_sin * magnitude);
}

inline void PhaseVocoder::PerformFFT(dsp::Complex<float>* input, dsp::Complex<float>* output, bool inverse)
{
    if (!inverse)
    {
        m_forward_fft.perform(input, output, false);
    }
    else
    {
        m_reverse_fft.perform(input, output, true);
    }
}

void PhaseVocoder::ApplyWindowFunction(float* input, dsp::Complex<float>* output, uint32_t count, uint32_t window_start)
{
    for (uint32_t window_pos = 0; window_pos < count; window_pos++)
    {
        output[window_pos].real(m_window_function[window_start + window_pos] * input[window_pos]);
        output[window_pos].imag(0.0f);
    }
}

void PhaseVocoder::ApplyCircularShift(const dsp::Complex<float>* input, dsp::Complex<float>* output, uint32_t segment_size)
{
    memcpy(output, input + segment_size / 2, sizeof(dsp::Complex<float>)*segment_size / 2);
    memcpy(output + segment_size / 2, input, sizeof(dsp::Complex<float>)*segment_size / 2);
}

void PhaseVocoder::ApplyCircularShift(const float* input, float* output, uint32_t segment_size)
{
    memcpy(output, input + segment_size / 2, sizeof(float)*segment_size / 2);
    memcpy(output + segment_size / 2, input, sizeof(float)*segment_size / 2);
}

void PhaseVocoder::WriteWindow(dsp::Complex<float>* input, dsp::Complex<float>* output, uint32_t count)
{
    for (uint32_t window_pos = 0; window_pos < count; window_pos++)
    {
        output[window_pos].real(output[window_pos].real() + input[window_pos].real());
    }
}

void PhaseVocoder::DSP(float* input, float* output, uint32_t buff_size, uint32_t channel)
{
    if (channel > 1)
    {
        return;
    }

    for (uint32_t j = FFT_SIZE; j < BUFFER_SIZE; j++)
    {
        m_output_buffer[channel][j] = 0.0f;
    }

    // Round up to the nearest whole window
    uint32_t segment_size = buff_size + (buff_size % FFT_SIZE);
    float * input_copy = new float[segment_size + FFT_SIZE];

    for (uint32_t i = 0; i < buff_size; i++)
    {
        input_copy[i] = input[i];
    }
    for (uint32_t i = buff_size; i < segment_size + FFT_SIZE; i++)
    {
        input_copy[i] = 0.0f;
    }

    // Loop through segments
    for (uint32_t i = 0; i < segment_size; i += FFT_SIZE)
    {
        for (uint32_t j = 0; j < FFT_SIZE; j++)
        {

            m_input_buffer[channel][j] = input_copy[j + i];
            m_complex_intermed_fw[channel][j] = dsp::Complex<float>(0.0f, 0.0f);
            m_complex_intermed_rv[channel][j] = dsp::Complex<float>(0.0f, 0.0f);
            m_complex_out[channel][j] = dsp::Complex<float>(0.0f, 0.0f);
        }

        for (uint32_t j = FFT_SIZE; j < BUFFER_SIZE; j++)
        {
            m_input_buffer[channel][j] = 0.0f;
            m_complex_intermed_fw[channel][j] = dsp::Complex<float>(0.0f, 0.0f);
            m_complex_intermed_rv[channel][j] = dsp::Complex<float>(0.0f, 0.0f);
            m_complex_out[channel][j] = dsp::Complex<float>(0.0f, 0.0f);
        }
        ProcessSegment(m_input_buffer[channel], m_complex_out[channel], FFT_SIZE, channel);

        for (uint32_t j = 0; j < FFT_SIZE; j++)
        {
            m_output_buffer[channel][i + j] = m_complex_out[channel][j].real();
        }
    }

    for (uint32_t i = 0; i < buff_size; i++)
    {
       output[i] = m_output_buffer[channel][i];
    }
    delete[] input_copy;
}

void PhaseVocoder::ProcessSegment(float* input_buffer, dsp::Complex<float> * output_buffer, uint32_t segment_size, uint32_t channel)
{
    for (uint32_t i = m_hop_size; i < m_window_size; i += m_hop_size)
    {
        ApplyProcessing(input_buffer, m_complex_intermed_fw[channel], m_complex_intermed_rv[channel], output_buffer, m_window_size - i, i, channel);
    }

    for (uint32_t i = 0; i < segment_size; i += m_hop_size)
    {
        ApplyProcessing(input_buffer + i, m_complex_intermed_fw[channel], m_complex_intermed_rv[channel], output_buffer + i, m_window_size, 0, channel);
    }
}

void PhaseVocoder::ApplyProcessing( float* input,
                                    dsp::Complex<float>* intermed_fw,
                                    dsp::Complex<float>* intermed_rv,
                                    dsp::Complex<float>* output,
                                    uint32_t count,
                                    uint32_t window_start, 
                                    uint32_t channel)
{
    dsp::Complex<float> buff[FFT_SIZE * 2];
	dsp::Complex<float> shift_buff[FFT_SIZE * 2];

    ApplyWindowFunction(input, buff, count, window_start);
	//ApplyCircularShift(buff, shift_buff, count);


#if (SCALING_FACTOR != 1)
    {
        float scaling_factor = (float)SCALING_FACTOR;
        for (uint32_t i = 0; i < FFT_SIZE * 2; i++)
        {
            intermed_fw[i] = dsp::Complex<float>(intermed_fw[i].real() / (scaling_factor),
                intermed_fw[i].imag() / (scaling_factor));
        }
    }
#endif
    Process(buff, intermed_fw, intermed_rv, FFT_SIZE, m_type, channel);

	//ApplyCircularShift(intermed_rv, shift_buff, count);

    // Commit data to output buffer, since it is windowed we can just add
    WriteWindow(intermed_rv, output, count);
}