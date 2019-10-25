/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public AudioAppComponent,
						public Slider::Listener
{					
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

	float interpolate();
	int update(AudioSampleBuffer &buffer, float input);

	void unlock();
	void lock();

	void Listener::sliderValueChanged(Slider* slider);
	void crossfade();
	void resample();
	void resample_end();
	void resampleUpdate(float input);
	void reset();
	void checkLen();

private:
	String file;
	AudioSampleBuffer fileBuffer;
	AudioSampleBuffer delayBuffer;
	AudioSampleBuffer sampleBuffer;
	

	Slider feedbackSlider, rateSlider, drywetSlider, inputSlider;
	ToggleButton unlockButton;
	ToggleButton resampleButton;
	TextButton resetButton{ "reset" };
	
	bool firstSwitch;
	bool resampleBool;
	int resampleLen;
	int resampleIndex;
	int resampleMax;


	float drywet;
	float inputLevel;

	float feedback;
	int delayRate = 18;
	int sample;
	int delayNum;
	int delayMax;
	int duration;

	AudioFormatManager audioFormatManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
