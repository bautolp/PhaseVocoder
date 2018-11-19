/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <vector>
#include <iostream>
#include <thread>       

float PhaseVocoder::m_effect;
ProcessType PhaseVocoder::m_type;
float PhaseVocoder::m_pitch_ratio;
uint32_t PhaseVocoder::m_bin_ratio[FFT_SIZE];
PhaseVocoder::slider_info PhaseVocoder::m_sliders[SLIDER_COUNT];

//==============================================================================
PhaseVocoderPluginAudioProcessor::PhaseVocoderPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", AudioChannelSet::stereo(), true)
#endif
	), treeState(*this, nullptr)
#endif
{

	NormalisableRange<float> effectRange(0.0f, 1.0f);
	treeState.createAndAddParameter("Robotization_Phase", "Robotization_Phase", "Robotization_Phase", effectRange, 1.0f, nullptr, nullptr);

    phase_vocoder = new PhaseVocoder();
}

PhaseVocoderPluginAudioProcessor::~PhaseVocoderPluginAudioProcessor()
{
    if (phase_vocoder)
    {
        delete phase_vocoder;
        phase_vocoder = NULL;
    }
}

//==============================================================================
const String PhaseVocoderPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhaseVocoderPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PhaseVocoderPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PhaseVocoderPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PhaseVocoderPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhaseVocoderPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PhaseVocoderPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhaseVocoderPluginAudioProcessor::setCurrentProgram (int index)
{
    (void)index;
}

const String PhaseVocoderPluginAudioProcessor::getProgramName (int index)
{
    (void)index;
    return {};
}

void PhaseVocoderPluginAudioProcessor::changeProgramName (int index, const String& newName)
{
    (void)index;
    (void)newName;
}

//==============================================================================
void PhaseVocoderPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    lowpass.prepare(spec);
    lowpass.reset();

}

void PhaseVocoderPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    phase_vocoder->Finish();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhaseVocoderPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PhaseVocoderPluginAudioProcessor::update_parameters()
{

}

void PhaseVocoderPluginAudioProcessor::dsp_process(dsp::ProcessContextReplacing<float> context)
{
    (void)context;
    // do processing here 
}

void PhaseVocoderPluginAudioProcessor::update_filter()
{
    float freq_ct = 800;
    int samp_rate = 44100;
    float resonance = 2;
    *lowpass.state = *dsp::IIR::Coefficients<float>::makeLowPass(samp_rate, freq_ct, resonance);
}

void PhaseVocoderPluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    (void)midiMessages;
    ScopedNoDenormals noDenormals;

    std::vector<std::thread> thrd;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float * output = buffer.getWritePointer(channel);
        thrd.push_back(std::thread(DSPThread, output, output, buffer.getNumSamples(), channel, (void*)phase_vocoder));
        //dsp_thread.join();
        ///float * output = buffer.getWritePointer(channel);
        //std::thread dsp_thread(DSPThread, output, output, buffer.getNumSamples(), channel, (void*)phase_vocoder);
        //dsp_thread.join();
//        thrd.push_back(std::thread(DSPThread, output, output, buffer.getNumSamples(), channel, (void*)phase_vocoder));
    }

    for (int channel = 0; channel < thrd.size(); ++channel)
    {
        thrd[channel].join();
    }
}

//==============================================================================
bool PhaseVocoderPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* PhaseVocoderPluginAudioProcessor::createEditor()
{
    return new PhaseVocoderPluginAudioProcessorEditor (*this);
}

//==============================================================================
void PhaseVocoderPluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    (void)destData;
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PhaseVocoderPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    (void)sizeInBytes;
    (void)data;
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhaseVocoderPluginAudioProcessor();
}

static void DSPThread(float* input, float* output, uint32_t buff_size, uint32_t channel, void * phase_vocoder)
{
    PhaseVocoder* pv = (PhaseVocoder*)phase_vocoder;
    pv->DSP(input, output, buff_size, channel);
}