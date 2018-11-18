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
    GenerateWindowFunction();
    for (uint32_t i = 0; i < THREAD_COUNT; i++)
    {
        for (uint32_t j = 0; j < FFT_SIZE * 2; j++)
        {
            m_input_buffer[i][j] = 0.0f;
            m_complex_intermed_fw[i][j] = dsp::Complex<float>(0.0f, 0.0f);
            m_complex_intermed_rv[i][j] = dsp::Complex<float>(0.0f, 0.0f);
            m_complex_out[i][j] = dsp::Complex<float>(0.0f, 0.0f);
        }
    }
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

void PhaseVocoder::Process(dsp::Complex<float> * fft_data, uint32_t fft_size, ProcessType type)
{
    switch (type)
    {
    case ProcessType::PitchShift:
        PitchShift(fft_data, fft_size);
        break;
    case ProcessType::Robotization:
        Robotization(fft_data, fft_size);
        break;
    case ProcessType::Whisperization:
        Whisperization(fft_data, fft_size);
        break;
    case ProcessType::Phaser:
        Phaser(fft_data, fft_size);
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

void PhaseVocoder::PitchShift(dsp::Complex<float>* fft_data, uint32_t fft_size)
{
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
        ApplyProcessing(input_buffer, m_complex_intermed_fw[channel], m_complex_intermed_rv[channel], output_buffer, m_window_size - i, i);
    }

    for (uint32_t i = 0; i < segment_size; i += m_hop_size)
    {
        ApplyProcessing(input_buffer + i, m_complex_intermed_fw[channel], m_complex_intermed_rv[channel], output_buffer + i, m_window_size, 0);
    }
}

void PhaseVocoder::ApplyProcessing( float* input,
                                    dsp::Complex<float>* intermed_fw,
                                    dsp::Complex<float>* intermed_rv,
                                    dsp::Complex<float>* output,
                                    uint32_t count,
                                    uint32_t window_start)
{
    dsp::Complex<float> buff[FFT_SIZE * 2];
	dsp::Complex<float> shift_buff[FFT_SIZE * 2];

    ApplyWindowFunction(input, buff, count, window_start);
	//ApplyCircularShift(buff, shift_buff, count);

    for (uint32_t i = count; i < FFT_SIZE * 2; i++)
    {
        buff[i] = dsp::Complex<float>(0.0f, 0.0f);
		shift_buff[i] = dsp::Complex<float>(0.0f, 0.0f);
    }
    for (uint32_t i = 0; i < FFT_SIZE * 2; i++)
    {
        intermed_fw[i] = dsp::Complex<float>(0.0f, 0.0f);
        intermed_rv[i] = dsp::Complex<float>(0.0f, 0.0f);
    }

    m_forward_fft.perform(buff, intermed_fw, false);

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
    Process(intermed_fw, FFT_SIZE, m_type);

    // Inverse FFT
    m_reverse_fft.perform(intermed_fw, intermed_rv, true);
	//ApplyCircularShift(intermed_rv, shift_buff, count);

    // Commit data to output buffer, since it is windowed we can just add
    WriteWindow(intermed_rv, output, count);
}

/*
static void DSPThread(float* input, float* output, uint32_t buff_size, uint32_t channel, void * phase_vocoder)
{
    PhaseVocoder* pv = (PhaseVocoder*)phase_vocoder;
    pv->DSP(input, output, buff_size, channel);
}
*/