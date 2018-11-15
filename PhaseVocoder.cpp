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

void PhaseVocoder::ApplyWindowFunction(dsp::Complex<float>* input, dsp::Complex<float>* output, uint32_t count, uint32_t window_start)
{
    for (uint32_t window_pos = 0; window_pos < count; window_pos++)
    {
        output[window_pos] = m_window_function[window_start + window_pos] * input[window_pos];
    }
}

void PhaseVocoder::ApplyCircularShift(const dsp::Complex<float>* input, dsp::Complex<float>* output, uint32_t segment_size)
{
	memcpy(output, input + segment_size / 2, sizeof(dsp::Complex<float>)*segment_size / 2);
	memcpy(output + segment_size / 2, input, sizeof(dsp::Complex<float>)*segment_size / 2);
}

void PhaseVocoder::PhaseLock()
{

}

void PhaseVocoder::WriteWindow(dsp::Complex<float>* input, dsp::Complex<float>* output, uint32_t count)
{
    for (uint32_t window_pos = 0; window_pos < count; window_pos++)
    {
        output[window_pos] += dsp::Complex<float>(  input[window_pos].real(),
                                                    input[window_pos].imag())*m_effect;
    }
}

void PhaseVocoder::DSP(float* input, float* output, uint32_t buff_size, uint32_t channel)
{
    for (uint32_t i = 0; i < buff_size; i++)
    {
        m_complex_in[channel][i] = dsp::Complex<float>(input[i], 0.0f);
        m_complex_out[channel][i] = dsp::Complex<float>(0.0f, 0.0f);
    }
    
    ProcessSegment(m_complex_in[channel], m_complex_out[channel], buff_size);

    for (uint32_t i = 0; i < buff_size; i++)
    {
        output[i] = m_complex_out[channel][i].real();
    }
}

template <typename Word>
std::ostream& write_word(std::ostream& outs, Word value, unsigned size = sizeof(Word))
{
    for (; size; --size, value >>= 8)
        outs.put(static_cast <char> (value & 0xFF));
    return outs;
}

void PhaseVocoder::Process(dsp::Complex<float> * fft_data, uint32_t fft_size, ProcessType type)
{
    switch (type)
    {
        case ProcessType::PitchShift:
            break;
        case ProcessType::Robotization:
            Robotization(fft_data, fft_size);
            break;
        case ProcessType::Whisperization:
            Whisperization(fft_data, fft_size);
            break;
        default:
            break;
    }
}
void PhaseVocoder::Whisperization(dsp::Complex<float> * fft_data, uint32_t fft_size) 
{

}

void PhaseVocoder::changeEffect(const float newValue)
{
	m_effect = newValue;
}

void PhaseVocoder::Robotization(dsp::Complex<float> * fft_data, uint32_t fft_size)
{

	for (uint16_t i = 0; i < fft_size; i++) {
		float magnitude = abs(fft_data[i]);
		fft_data[i].imag(0.0f);
		fft_data[i].real(magnitude/**cosf((2 * M_PI)*m_robotization_phase)*/);
	}
}

void PhaseVocoder::ProcessSegment(dsp::Complex<float>* input_buffer, dsp::Complex<float> * output_buffer, uint32_t segment_size)
{
    // Too small
	if (segment_size <= m_window_size)
	{
		if (segment_size <= m_window_size / 2)
		{
			ApplyProcessing(input_buffer, m_complex_intermed_fw, m_complex_intermed_rv, output_buffer, segment_size, 0);
			ApplyProcessing(input_buffer, m_complex_intermed_fw, m_complex_intermed_rv, output_buffer, segment_size, m_window_size / 2);
		}
		else
		{
			ApplyProcessing(input_buffer, m_complex_intermed_fw, m_complex_intermed_rv, output_buffer, segment_size, 0);
			ApplyProcessing(input_buffer, m_complex_intermed_fw, m_complex_intermed_rv, output_buffer, m_window_size / 2, m_window_size / 2);
			ApplyProcessing(input_buffer, m_complex_intermed_fw, m_complex_intermed_rv, output_buffer, segment_size - m_window_size / 2, 0);
		}
		return;
	}

    for (uint32_t i = m_hop_size; i < m_window_size; i += m_hop_size)
    {
        ApplyProcessing(input_buffer, m_complex_intermed_fw, m_complex_intermed_rv, output_buffer, WINDOW_SIZE - i, i);
    }

    for (uint32_t i = 0; i < segment_size; i += m_hop_size)
    {
        uint32_t window_size = (segment_size - i) > WINDOW_SIZE ? WINDOW_SIZE : segment_size - i;
        ApplyProcessing(input_buffer + i, m_complex_intermed_fw, m_complex_intermed_rv, output_buffer + i, window_size, 0);
    }
}

void PhaseVocoder::ApplyProcessing(dsp::Complex<float>* input,
    dsp::Complex<float>* intermed_fw,
    dsp::Complex<float>* intermed_rv,
    dsp::Complex<float>* output,
    uint32_t count,
    uint32_t window_start)
{
    float scaling_factor = (float)SCALING_FACTOR / 4;
    dsp::Complex<float> buff[FFT_SIZE];
	dsp::Complex<float> shift_buff[FFT_SIZE];

    ApplyWindowFunction(input, buff, count, window_start);
	ApplyCircularShift(buff, shift_buff, count);

    for (uint32_t i = count; i < FFT_SIZE; i++)
    {
		shift_buff[i] = dsp::Complex<float>(0.0f, 0.0f);
    }

    m_forward_fft.perform(shift_buff, intermed_fw, false);

    if ((int)scaling_factor != 1)
    {
        for (uint32_t i = 0; i < FFT_SIZE; i++)
        {
            intermed_fw[i] = dsp::Complex<float>(   intermed_fw[i].real() / (scaling_factor),
                                                    intermed_fw[i].imag() / (scaling_factor));
        }
    }

    Process(intermed_fw, FFT_SIZE, ProcessType::Robotization);

    // Perform Phase locking (currently does nothing)
    PhaseLock();

    // Inverse FFT
    m_reverse_fft.perform(intermed_fw, intermed_rv, true);
	ApplyCircularShift(intermed_rv, shift_buff, count);

    // Scale buffer back
    //ReScaleWindow(intermed, count, (float)FFT_SIZE);

    // Commit data to output buffer, since it is windowed we can just add
    WriteWindow(shift_buff, output, count);
}