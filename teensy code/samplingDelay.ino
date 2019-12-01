//the satellites are spinning
//a new day is dawning
//the galaxies are waiting
//for planet Earth's awakening


#include <Bounce.h>

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

float feedback = .8;
short int input = 0;
const short int delayMax = 15000;
short int sampleBuffer [delayMax];
float delayBuffer[delayMax];
short int delayNum = 0;
short int delayRate = 0;
bool sampleBool = false;
short int sampleNum = 0;
short int sampleLen = delayMax;
short int outputBuffer[256];
float sub = 0;
float knockDown = -.2;

bool resetBool = false;

int samplerate = 0;
short int sampleButton = 2;
short int resetButton = 8;
int modCount = 0;

IntervalTimer outputTimer;
IntervalTimer inputTimer;
int counter = 0;

void setup() {
  //uint32_t freq = 22058;  // try a sampling rate of 22.058 kHz (44.117 kHz is base, so we'll do half)
  //pin 7,20,21 used by DAC
  //aka A6, A7
  pinMode(sampleButton, INPUT);
  pinMode(resetButton, INPUT);

  outputTimer.begin(outInterrupt, 45.334);
  inputTimer.begin(inInterrupt, 400);

  adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED:: HIGH_SPEED);
  adc->setAveraging(32);
  adc->setResolution(12);

  AudioMemory(60);

  for (short int i = 0; i < delayMax; i++) {
    delayBuffer[i] = 0;
    sampleBuffer[i] = 0;
  }

}


void outInterrupt() {
  cli();
  //adc->adc0->readSingle();
  input = (adc->analogRead(A2)) * 2;
  if (counter == 128) {
    getNextOutputBlock();
    counter = 0;
  }
  outputBuffer[counter] = input;
  outputBuffer[counter + 1] = input;
  counter += 2;
  sei();
}

void inInterrupt() {
  //Note: pins A7 and A6 are used by DAC
  //aka 20 and 21
  feedback = ((float)(adc->analogRead(A5)) / 3350);
  delayRate = map(adc->analogRead (A4), 0, 8535, 1, 35);
  samplerate = map(adc->analogRead (A9), 0, 8535, 1, 18);
  sampleBool = digitalRead(sampleButton);
  resetBool = digitalRead(resetButton);
}

void getNextOutputBlock() {

  //quickly update sample and delay buffers, then update output buffer and pass it to DAC
  for (int i = 0; i < 128; i += 2) {
    checkLen();
    if (sampleNum >= delayMax)
      sampleNum = sampleNum - delayMax;
    short int holdNum = outputBuffer[i];
    outputBuffer[i] = (outputBuffer[i] + delayBuffer[delayNum]) * 2 + sampleBuffer[sampleNum];
    //outputBuffer[i] = outputBuffer[i];
    outputBuffer[i + 1] = outputBuffer[i];

    updateDelay(holdNum);
    if (sampleBool) {
      updateSample(delayBuffer[delayNum]);
    }
    else
      sampleNum += samplerate;
    sampleBuffer[sampleNum] -= sampleBuffer[sampleNum] * sub;
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
    if(in > 0)
       knockDown = .3;
    else
      knockDown = 0;
      
    delayBuffer[delayNum] = .8 * ((float)(feedback-knockDown) * delayBuffer[delayNum] + in);
    delayNum++;
  }
}


void updateSample(short int in) {
  for (short int i = 0; i < samplerate; i++) {
    if (sampleNum >= delayMax)
      sampleNum = sampleNum - delayMax;
    
    if(in > 0)
       knockDown = .3;
    else
      knockDown = 0;

    sampleBuffer[sampleNum] = .8*((float)(1-knockDown)*sampleBuffer[sampleNum] + in);
    sampleNum++;
  }
}

void resetBuffer() {
  for (int i = 0; i < delayMax; i++) {
    sampleBuffer[i] = 0;
  }
}

void loop() {
  if(resetBool){
    resetBuffer();9
  }
}
