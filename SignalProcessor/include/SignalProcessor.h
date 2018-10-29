#ifndef SIGNALPROCESSOR_H
#define SIGNALPROCESSOR_H

#include <string>
#include <vector>

#ifdef SIGNALPROCESSOR_EXPORTS
#define SIGNALPROCESSOR_API __declspec(dllexport)
#else
#define SIGNALPROCESSOR_API __declspec(dllimport)
#endif

typedef double complex_interface[2];

class SignalProcessor
{
public:

    SIGNALPROCESSOR_API SignalProcessor();
    SIGNALPROCESSOR_API ~SignalProcessor();

    SIGNALPROCESSOR_API virtual complex_interface* ForwardFFT(complex_interface* input) = 0;
    SIGNALPROCESSOR_API virtual complex_interface* InverseFFT(complex_interface* input) = 0;
    SIGNALPROCESSOR_API virtual void ReleaseFFT(complex_interface* input) = 0;

    SIGNALPROCESSOR_API virtual void Disconnect() = 0;
};

// Create function pointer, used in Factory.
typedef SignalProcessor*(__stdcall* CreateSignalProcessorFn)(uint32_t sample_size);

#endif // THERMOCOUPLE_H
