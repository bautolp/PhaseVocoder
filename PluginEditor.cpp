/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Images.h"

//==============================================================================
PhaseVocoderPluginAudioProcessorEditor::PhaseVocoderPluginAudioProcessorEditor (PhaseVocoderPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
	m_background_image = ImageCache::getFromMemory(Images::background_jpg, Images::background_jpgSize);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
	//slider initialization
	sliderAttach = new AudioProcessorValueTreeState::SliderAttachment(processor.treeState, "Robotization_Phase", phaseSlider);
	phaseSlider.setSliderStyle(Slider::SliderStyle::Rotary);
	phaseSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 100, 25);
	phaseSlider.setRange(0.0f, 1.0f);
	phaseSlider.setValue(PhaseVocoder::m_effect);
	phaseSlider.setColour(Slider::ColourIds::thumbColourId, Colour::fromRGB(150, 0, 175));
	phaseSlider.setColour(Slider::ColourIds::trackColourId, Colour::fromRGB(100, 0, 125));
	phaseSlider.addListener(this);
	addAndMakeVisible(phaseSlider);

	setSize(m_width, m_hieght);
}

PhaseVocoderPluginAudioProcessorEditor::~PhaseVocoderPluginAudioProcessorEditor()
{
}

//==============================================================================
void PhaseVocoderPluginAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
	g.drawImage(m_background_image, 0, 0, m_width, m_hieght, 0, 0, m_background_image.getWidth(), m_background_image.getHeight());
	g.setColour(Colour(216, 255, 224));
	g.setFont(Font("Lucida Console", m_font_title_size, Font::FontStyleFlags::plain));
	g.drawFittedText("Vocodinator", (m_width) / 4, m_vertical_top_padding, m_width / 2, m_hieght / 8, Justification::centredTop, 1, 1);
}

void PhaseVocoderPluginAudioProcessorEditor::log(Graphics& g, std::string msg)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    g.setColour(Colours::white);
    g.setFont(15.0f);
    g.drawFittedText(msg, getLocalBounds(), Justification::centred, 1);
}

void PhaseVocoderPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	phaseSlider.setBounds(juce::Rectangle<int>(100, 100, 150, 150));
}

	void PhaseVocoderPluginAudioProcessorEditor::sliderValueChanged(Slider *slider)
	{
		/*This is where the slider changes the variable*/
		if (slider == &phaseSlider) {
			//std::cout << phaseSlider.getValue() << std::endl;
			PhaseVocoder::changeEffect(phaseSlider.getValue());
		}

	}
