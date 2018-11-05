/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class PhaseVocoder
{
public:
    PhaseVocoder();
    ~PhaseVocoder();
private:
    void DSPProcessing(float* input, float* output, uint32_t hop_size, uint32_t buff_size);
};
