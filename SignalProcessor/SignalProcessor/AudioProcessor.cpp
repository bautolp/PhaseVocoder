#include "AudioProcessor.h"
#include "fftw3.h"
#include <vector>


AudioProcessor::AudioProcessor(uint32_t sample_size)
{
    m_sample_size = sample_size;
}

AudioProcessor::~AudioProcessor()
{
}


complex_interface* AudioProcessor::ForwardFFT(complex_interface* input)
{
    fftw_complex *in = input;
    fftw_complex* out;
    fftw_plan p;

    // Output signal
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * m_sample_size);
    p = fftw_plan_dft_1d(m_sample_size, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    fftw_execute(p); 

    fftw_destroy_plan(p);
    return out;
}

complex_interface* AudioProcessor::InverseFFT(complex_interface* input)
{
    fftw_complex *in = input;
    fftw_complex* out;
    fftw_plan p;

    // Output signal
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * m_sample_size);
    p = fftw_plan_dft_1d(m_sample_size, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

    fftw_execute(p); /* repeat as needed */

    fftw_destroy_plan(p);
    return out;
}

void AudioProcessor::ReleaseFFT(complex_interface* input)
{
    fftw_free((fftw_complex*)input);
}

void AudioProcessor::Disconnect()
{
    delete this;
}
