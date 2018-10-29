#ifndef AUDIOPROCESSOR_H
#define AUDIOPROCESSOR_H

#include "SignalProcessor.h"
#include <stdint.h>
#include <string>

class AudioProcessor : public SignalProcessor
{
public:
    AudioProcessor(uint32_t sample_size);
    ~AudioProcessor();

    complex_interface* ForwardFFT(complex_interface* input);
    complex_interface* InverseFFT(complex_interface* input);
    void ReleaseFFT(complex_interface* input);
    void Disconnect();

    static SignalProcessor* __stdcall Create(uint32_t sample_size)
    {
        return new AudioProcessor(sample_size);
    }

private:
    uint32_t m_sample_size;
};

#endif // PAPAGO_H
