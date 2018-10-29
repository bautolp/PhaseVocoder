// SignalProcessorTest.cpp : Defines the entry point for the console application.
//

// clang-format off
#include "stdafx.h"
#include <iostream>
#include "SignalProcessorFactory.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
// clang-format on

#define N 8

int main() {
    // Generate cosine
    complex_interface in[N];

    for (int i = 0; i < N; i++) 
    {
        in[i][0] = cos((8 * 2 * M_PI*i) / N);
        std::cout << "in " << in[i][0] << std::endl;
        in[i][1] = 0;
    }

    // Create signal processor
    SignalProcessor* signal_processor = SignalProcessorFactory::Get()->CreateSignalProcessor(N);

    // Perform FFT
    complex_interface * fft = signal_processor->ForwardFFT(in);
    complex_interface * inverse_fft = signal_processor->InverseFFT(fft);
    
    for (int i = 0; i < N; i++)
    {
        std::cout << "freq " << i << ": " << fft[i][0] << ", " << fft[i][1] << std::endl;
    }

    // Normalize
    for (int i = 0; i < N; i++) {
        inverse_fft[i][0] *= 1. / N;
        inverse_fft[i][1] *= 1. / N;
    }

    for (int i = 0; i < N; i++)
    {
        std::cout << "Initial " << i << ": " << in[i][0] << ", " << in[i][1] << std::endl;
        std::cout << "inverse " << i << ": " << inverse_fft[i][0] << ", " << inverse_fft[i][1] << std::endl;
    }

    // Release memory 
    signal_processor->ReleaseFFT(fft);
    signal_processor->ReleaseFFT(inverse_fft);

    return 0;
}

