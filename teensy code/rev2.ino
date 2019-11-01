#include <ADC.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

ADC *adc  = new ADC();

// GUItool: begin automatically generated code
AudioPlayQueue           outputQueue;         //xy=733,426
AudioOutputPT8211        pt8211_1;       //xy=919,423
AudioConnection          patchCord2(outputQueue, 0, pt8211_1, 0);
AudioConnection          patchCord3(outputQueue, 0, pt8211_1, 1);
// GUItool: end automatically generated code*/

float feedback = 0.8;
short int input = 0;
const short int delayMax = 3000;
short int sampleBuffer [delayMax];
short int delayBuffer[delayMax];
short int delayNum = 0;
short int delayRate = 1;
bool sampleBool = false;
short int sampleNum = 0;
short int sampleLen = delayMax;
short int outputBuffer[256];

IntervalTimer sampleOut;
int counter = 0;

void setup() {
  uint32_t freq = 22058;  // try a sampling rate of 22.058 kHz (44.117 kHz is base, so we'll do half)

  adc->adc0->stopPDB();
  adc->adc0->startSingleRead(A1);
  adc->enableInterrupts(ADC_0);
  adc->adc0->startPDB(freq); //frequency in Hz

  adc->adc1->stopPDB();
  adc->adc1->startSingleRead(A9);
  adc->enableInterrupts(ADC_1);
  adc->adc1->startPDB(freq); //frequency in Hz


  AudioMemory(20);

  for (short int i = 0; i < delayMax; i++) {
    delayBuffer[i] = 0;
    sampleBuffer[i] = 0;
  }

  adc->setConversionSpeed(ADC_CONVERSION_SPEED:: HIGH_SPEED);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED:: HIGH_SPEED);
  adc->setAveraging(32);
  adc->setResolution(12);

  adc->setAveraging(16, ADC_1); // set number of averages
  adc->setResolution(12, ADC_1); // set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_1); // change the conversion speed
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_1); // change the sampling speed

}


void adc0_isr() {
  cli();
  //adc->adc0->readSingle();
  input = (adc->readSingle(A1)) * 2;
  if (counter == 128) {
    getNextOutputBlock();
    counter = 0;
  }
  outputBuffer[counter] = input;
  outputBuffer[counter + 1] = input;
  counter += 2;
  sei();
}

void adc1_isr() {
  //cli();
  adc->adc1->readSingle(); 
  feedback = (float)(adc->readSingle(A9)) / (float)8000;
  //sei();
}

void getNextOutputBlock() {

  //quickly update sample and delay buffers, then update output buffer and pass it to DAC
  for (int i = 0; i < 128; i += 2) {
    short int holdNum = outputBuffer[i];
    outputBuffer[i] = outputBuffer[i] + delayBuffer[delayNum];
    //outputBuffer[i] = outputBuffer[i];
    outputBuffer[i + 1] = outputBuffer[i];

    updateDelay(holdNum);
  }

  int16_t *p = outputQueue.getBuffer();
  memcpy(p, outputBuffer, 256);
  outputQueue.playBuffer();
}


void checkLen() {
  if (delayNum >= delayMax) {
    delayNum = delayNum - delayMax;
  }

  if (sampleNum >= sampleLen) {
    sampleNum = sampleNum - sampleLen;
  }
}

void updateDelay(short int in) {
  for (short int i = 0; i < delayRate; i++) {
    if (delayNum >= delayMax)
      delayNum = delayNum - delayMax;
    delayBuffer[delayNum] = ((float)feedback * (float)delayBuffer[delayNum] + in);
    delayNum++;
  }
}

void updateSample() {
  for (short int i = 0; i < delayMax; i++) {
    if (sampleNum == delayMax)
      sampleNum = 0;
    sampleBuffer[sampleNum] = delayBuffer[delayNum] + sampleBuffer[sampleNum];
    sampleNum++;
  }
}

void loop() {

}
