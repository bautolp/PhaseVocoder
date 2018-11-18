/*
==============================================================================

This file was auto-generated!

It contains the basic framework code for a JUCE plugin editor.

==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PhaseVocoderPluginAudioProcessorEditor : public AudioProcessorEditor, public Slider::Listener, public ComboBox::Listener
{
public:
	PhaseVocoderPluginAudioProcessorEditor(PhaseVocoderPluginAudioProcessor&);
	~PhaseVocoderPluginAudioProcessorEditor();

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;
	void log(Graphics&, std::string);
    void sliderValueChanged(Slider *slider) override;
    void comboBoxChanged(ComboBox *comboBox) override;
    void PhaseVocoderChanged();
	ScopedPointer <AudioProcessorValueTreeState::SliderAttachment> sliderAttach;
private:

	const int m_width = 600;
	const int m_hieght = 600;
	const int m_hoizontal_padding = 50;
	const int m_vertical_top_padding = 20;
	const int m_vertical_bottom_padding = 50;
	const float m_font_title_size = 40.0f;
	Image m_background_image;

	PhaseVocoderPluginAudioProcessor& processor;
	Slider phaseSlider;
    ComboBox m_phase_vocoder_type;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseVocoderPluginAudioProcessorEditor)
};
