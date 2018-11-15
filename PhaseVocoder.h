/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

enum WindowFunctionType
{
    Flat,
    Hanning
};

enum ProcessType
{
    PitchShift,
    Robotization,
    Whisperization
};

#define FFT_ORDER 9
#define FFT_SIZE (1 << FFT_ORDER)
#define SEGMENT_SIZE 256
#define WINDOW_SIZE 512
#define HOP_SIZE 256
#define SCALING_FACTOR ((FFT_SIZE / WINDOW_SIZE) * (WINDOW_SIZE / (HOP_SIZE*2)))

#define INTERM_BUFFER_SIZE 8192


class PhaseVocoder
{
public:
    PhaseVocoder();
    ~PhaseVocoder();
    void DSP(float* input, float* output, uint32_t buff_size, uint32_t channel);
    void Finish();
	static float m_effect;
	static void changeEffect(const float newValue);

private:

    // Allocate output buffer and tmp buffer
    uint32_t m_buffer_size[2] = { 0, 0 };

    uint32_t m_buffer_size_online[2] = { 0, 0 };

    dsp::Complex<float> m_complex_intermed_fw[FFT_SIZE];
    dsp::Complex<float> m_complex_intermed_rv[FFT_SIZE];
    dsp::Complex<float> m_complex_in[2][FFT_SIZE * 2];
    dsp::Complex<float> m_complex_out[2][FFT_SIZE * 2];
    uint32_t m_window_size = WINDOW_SIZE;
    uint32_t m_hop_size = HOP_SIZE;
    dsp::FFT m_forward_fft;
    dsp::FFT m_reverse_fft;
    float m_window_function[WINDOW_SIZE];

    void ProcessSegment(dsp::Complex<float>* input_buffer, dsp::Complex<float> * output_buffer, uint32_t segment_size);
    void ApplyProcessing(dsp::Complex<float>* input, dsp::Complex<float>* intermed_fw, dsp::Complex<float>* intermed_rv, dsp::Complex<float>* output, uint32_t count, uint32_t window_start);
    void ApplyWindowFunction(dsp::Complex<float>* input, dsp::Complex<float>* output, uint32_t count, uint32_t window_start);
    void GenerateWindowFunction();
	void ApplyCircularShift(const dsp::Complex<float>* input, dsp::Complex<float>* output, uint32_t segment_size);

    void Process(dsp::Complex<float> * fft_data, uint32_t fft_size, ProcessType type);
    void PhaseLock();
    void WriteWindow(dsp::Complex<float>* input, dsp::Complex<float>* output, uint32_t count);
    void Whisperization(dsp::Complex<float> * fft_data, uint32_t fft_size);
    void Robotization(dsp::Complex<float> * fft_data, uint32_t fft_size);
};
