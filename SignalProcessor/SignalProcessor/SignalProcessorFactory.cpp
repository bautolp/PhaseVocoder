#include "AudioProcessor.h"
#include "SignalProcessorFactory.h"

using namespace std;

SignalProcessorFactory::SignalProcessorFactory()
{
    RegisterSignalProcessor("AudioProcessor", &AudioProcessor::Create);
}

void SignalProcessorFactory::RegisterSignalProcessor(const string& SignalProcessor_type, CreateSignalProcessorFn create_fn)
{
    m_SignalProcessor_factory_map[SignalProcessor_type] = create_fn;
}

SignalProcessor* SignalProcessorFactory::CreateSignalProcessor(uint32_t sample_size)
{
    for (SignalProcessorMap::iterator it = m_SignalProcessor_factory_map.begin(); it != m_SignalProcessor_factory_map.end(); ++it)
    {
        SignalProcessor* SignalProcessor = NULL;
        try
        {
            SignalProcessor = it->second(sample_size);
            return SignalProcessor;
        }
        catch (...) { }

        if (SignalProcessor != NULL)
        {
            SignalProcessor->Disconnect();
            SignalProcessor = NULL;
        }
    }
    return NULL;
}
