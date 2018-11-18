/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

enum ProcessType
{
    PitchShift,
    Robotization,
    Whisperization,
    Phaser,
    NoneDebug
};

#define FFT_ORDER 9
#define FFT_SIZE (1 << FFT_ORDER)
#define WINDOW_SIZE 256
#define HOP_SIZE 128
#define SCALING_FACTOR (WINDOW_SIZE / (HOP_SIZE*2))

#define THREAD_COUNT 2
#define BUFFER_SIZE FFT_SIZE * 2

class PhaseVocoder
{
public:
    PhaseVocoder();
    ~PhaseVocoder();
    void DSP(float* input, float* output, uint32_t buff_size, uint32_t channel);
    void Finish();
    static float m_effect;
    static float m_pitch_ratio;
    static void change_effect(const float new_value);
    static ProcessType m_type;
    static void change_type(const ProcessType new_type);

private:

    // Allocate output buffer and tmp buffer
    uint32_t m_buffer_size[2] = { 0, 0 };

    uint32_t m_buffer_size_online[2] = { 0, 0 };

    dsp::Complex<float> m_complex_intermed_fw[THREAD_COUNT][BUFFER_SIZE];
    dsp::Complex<float> m_complex_intermed_rv[THREAD_COUNT][BUFFER_SIZE];
    dsp::Complex<float> m_complex_out[THREAD_COUNT][BUFFER_SIZE];
    float m_last_phase[THREAD_COUNT][FFT_SIZE];
    float m_psi[THREAD_COUNT][FFT_SIZE];
    uint32_t m_window_size = WINDOW_SIZE;
    uint32_t m_hop_size = HOP_SIZE;
    dsp::FFT m_forward_fft;
    dsp::FFT m_reverse_fft;
    float m_window_function[WINDOW_SIZE];
    float m_input_buffer[THREAD_COUNT][BUFFER_SIZE];
    float m_output_buffer[THREAD_COUNT][BUFFER_SIZE];

    void ProcessSegment(float* input_buffer, dsp::Complex<float> * output_buffer, uint32_t segment_size, uint32_t channel);
    void ApplyProcessing(float* input, dsp::Complex<float>* intermed_fw, dsp::Complex<float>* intermed_rv, dsp::Complex<float>* output, uint32_t count, uint32_t window_start, uint32_t channel);
    void ApplyWindowFunction(float* input, dsp::Complex<float>* output, uint32_t count, uint32_t window_start);
    void GenerateWindowFunction();
    void ApplyCircularShift(const float* input, float* output, uint32_t segment_size);
	void ApplyCircularShift(const dsp::Complex<float>* input, dsp::Complex<float>* output, uint32_t segment_size);

    void Process(dsp::Complex<float> * time_domain_input, dsp::Complex<float> * frequency_domain, dsp::Complex<float> * time_domain_output, uint32_t fft_size, ProcessType type, uint32_t channel);
    void PhaseLock();
    void WriteWindow(dsp::Complex<float>* input, dsp::Complex<float>* output, uint32_t count);
    void Whisperization(dsp::Complex<float> * fft_data, uint32_t fft_size);
    void Robotization(dsp::Complex<float> * fft_data, uint32_t fft_size);
    void PitchShift(dsp::Complex<float> * fft_data, uint32_t fft_size, uint32_t channel);
    void Phaser(dsp::Complex<float> * fft_data, uint32_t fft_size);
    inline void PerformFFT(dsp::Complex<float>* input, dsp::Complex<float>* output, bool inverse);
};

