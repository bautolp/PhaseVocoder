#ifndef SignalProcessorFACTORY_H
#define SignalProcessorFACTORY_H

#include "SignalProcessor.h"
#include <map>
#include <string>

#ifdef SIGNALPROCESSOR_EXPORTS
#define SIGNALPROCESSORFACTORY_API __declspec(dllexport)
#else
#define SIGNALPROCESSORFACTORY_API __declspec(dllimport)
#endif

class SignalProcessorFactory
{
public:
    SIGNALPROCESSORFACTORY_API ~SignalProcessorFactory()
    {
        m_SignalProcessor_factory_map.clear();
    }
    SIGNALPROCESSORFACTORY_API SignalProcessor* CreateSignalProcessor(uint32_t sample_size);
    SIGNALPROCESSORFACTORY_API static SignalProcessorFactory* Get()
    {
        static SignalProcessorFactory instance;
        return &instance;
    }

private:
    SignalProcessorFactory();
    SignalProcessorFactory(const SignalProcessorFactory&)
    {
    }
    SignalProcessorFactory& operator=(const SignalProcessorFactory&)
    {
        return *this;
    }
    void RegisterSignalProcessor(const std::string& SignalProcessor_type, CreateSignalProcessorFn create_fn);

    typedef std::map<std::string, CreateSignalProcessorFn> SignalProcessorMap;
    SignalProcessorMap m_SignalProcessor_factory_map;
};
#endif
