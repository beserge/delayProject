#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	setSize (800, 600);
	
	rateSlider.setRange(1, 40);
	rateSlider.addListener(this);
	addAndMakeVisible(rateSlider);

	feedbackSlider.setRange(0, 1);
	feedbackSlider.addListener(this);
	addAndMakeVisible(feedbackSlider);

	drywetSlider.setRange(0, 1);
	drywetSlider.addListener(this);
	addAndMakeVisible(drywetSlider);

	inputSlider.setRange(0, 1);
	inputSlider.addListener(this);
	addAndMakeVisible(inputSlider);

	unlockButton.onClick = [this]() { unlock(); };
	addAndMakeVisible(unlockButton);

	resampleButton.onClick = [this]() { resample(); };
	addAndMakeVisible(resampleButton);

	resetButton.onClick = [this]() { reset(); };
	addAndMakeVisible(resetButton);

    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { if (granted)  setAudioChannels (2, 2); });
    }
    else
    {
        setAudioChannels (2, 2);
    }
}

void MainComponent::sliderValueChanged(Slider* slider) {
	if (slider == &rateSlider) {
		delayRate = rateSlider.getValue();
	}

	else if (slider == &feedbackSlider) {
		feedback = feedbackSlider.getValue();
	}

	else if (slider == &drywetSlider) {
		drywet = drywetSlider.getValue();
	}

	else if (slider == &inputSlider) {
		inputLevel = inputSlider.getValue();
	}
}

void MainComponent::unlock() {
	if (unlockButton.getToggleState() == false)
		lock();
	else {

	}
}

void MainComponent::lock() {

}

void MainComponent::resample() {
	if (resampleButton.getToggleState() == false)
		resample_end();
	else {
		resampleBool = true;
		if (resampleLen == 0)
			firstSwitch = true;
	}
}

void MainComponent::resample_end() {
	//stop resampling and reset delaybuffer
	resampleBool = false;
	firstSwitch = false;
}

void MainComponent::reset() {
	resampleBool = false;
	firstSwitch = true;
	for (int i = 0; i < delayMax; i++) {
		delayBuffer.setSample(0, i, 0.00);
	}
	for (int i = 0; i < resampleMax; i++) {
		sampleBuffer.setSample(0, i, 0.00);
	}
	resampleIndex = 0;
	resampleLen = 0;
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
	file = "C:\\Users\\Ben\\Desktop\\metronome.wav";
	audioFormatManager.registerBasicFormats();
	std::unique_ptr<AudioFormatReader> reader(audioFormatManager.createReaderFor(file));
	duration = reader->lengthInSamples;
	fileBuffer.setSize(reader->numChannels, ((int)reader->lengthInSamples));
	reader->read(&fileBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
	sample = 0;

	feedback = 0.5;
	inputLevel = 1;
	delayNum = 0;
	delayMax = 100000;
	delayRate = 25;
	drywet = .5;

	delayBuffer.setSize(1, delayMax);
	for (int i = 0; i < delayMax; i++) {
		delayBuffer.setSample(0, i, 0.00);
	}
	
	firstSwitch = true;
	resampleLen = 0;
	resampleIndex = 0;
	resampleBool = false;
	resampleMax = 500000;
	sampleBuffer.setSize(1, resampleMax);
	for (int i = 0; i < resampleMax; i++) {
		sampleBuffer.setSample(0, i, 0.00);
	}
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
	auto* const leftIn = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
	auto* const rightIn = bufferToFill.buffer->getReadPointer(1, bufferToFill.startSample);

		float* const leftSpeaker = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
		float* const rightSpeaker = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

		for (int i = 0; i < bufferToFill.numSamples; ++i) {
			checkLen();

			leftSpeaker[i] = rightSpeaker[i] = (1 - drywet)*fileBuffer.getSample(0, sample) + (drywet)*delayBuffer.getSample(0, delayNum) + (drywet)*sampleBuffer.getSample(0, resampleIndex);

			//update(delayBuffer, inputLevel * fileBuffer.getSample(0, sample) + inputLevel * sampleBuffer.getSample(0,resampleIndex));
			
			//sample buffer into delay causes feedback loop!, but its a cool idea, if we can kill the feedback
			update(delayBuffer, inputLevel * fileBuffer.getSample(0, sample) );

			if (resampleBool)
				resampleUpdate(leftSpeaker[i]);

			sample++;
			resampleIndex++;
		}
}

void MainComponent::checkLen() {
	if (sample >= duration)
		sample = 0;

	if (delayNum >= delayMax)
		delayNum = delayNum - delayMax;

	if (resampleLen == 0)
		resampleIndex = 0;
	else if (resampleIndex >= resampleLen)
		resampleIndex = resampleIndex - resampleLen;
}

int MainComponent::update(AudioSampleBuffer &buffer, float input) {
	int ret = delayNum;
	for (int i = 0; i < delayRate; i++) {
		if (delayNum == delayMax)
			delayNum = 0;
		buffer.setSample(0, delayNum, (input +feedback * buffer.getSample(0, delayNum)));
		delayNum++;
	}
	return ret;
}

void MainComponent::resampleUpdate(float input) {
	if (firstSwitch) {
		if (resampleLen == resampleMax -1) {
			firstSwitch = false;
		}
		else {
			resampleLen++;
		}
	}

	sampleBuffer.setSample(0, resampleIndex, input);
}


void MainComponent::crossfade() {
	
}




float MainComponent::interpolate() {
	return 0;
}

void MainComponent::releaseResources()
{
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}


void MainComponent::resized()
{
	rateSlider.setBounds(100, 10, 600, 100);
	feedbackSlider.setBounds(100, 110, 600, 100);
	drywetSlider.setBounds(100, 210, 600, 100);
	inputSlider.setBounds(100, 310, 600, 100);
	unlockButton.setBounds(100, 410, 100, 100);
	resampleButton.setBounds(300, 410, 100, 100);
	resetButton.setBounds(500, 410, 100, 100);
}