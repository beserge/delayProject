#include <ADC.h>

IntervalTimer sampleOut;

byte x = 0;
const int readPin = A1; //adc
ADC *adc  = new ADC();

int delayNum = 0;
const int delayMax = 2000;
byte delayBuffer [delayMax];
int feedback = 186;
int delayRate = 1;
int math;

bool sampleBool = false;
byte sampleBuffer [delayMax];
int sampleNum = 0;

void setup() {
  //status LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(18, INPUT);

  analogWriteResolution(8);
  sampleOut.begin(outInterrupt, 200);

  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED:: VERY_HIGH_SPEED);
  adc->setAveraging(16);
  adc->setResolution(8);

  for (int i = 0; i < delayMax; i++) {
    delayBuffer[i] = 0;
  }
  for (int i = 0; i < delayMax; i++) {
    sampleBuffer[i] = 0;
  }
}

void outInterrupt() {
  if (delayNum >= delayMax)
    delayNum = delayNum - delayMax;
  if (sampleNum >= delayMax)
    sampleNum = sampleNum - delayMax;

  x = adc ->analogRead(A2);
  analogWrite(A12, sampleBuffer[sampleNum]/2 + delayBuffer[delayNum]/2);
  //analogWrite(A12, feedback);
  update();
  if (sampleBool)
    updateSample();
  sampleNum++;
}

void update() {
  for (int i = 0; i < delayRate; i++) {
    if (delayNum == delayMax)
      delayNum = 0;

    math = ((delayBuffer[delayNum] * feedback) / 255);
    math += x;
    delayBuffer[delayNum] = math/2;
    delayNum++;
  }
}

void updateSample() {
  math = (sampleBuffer[sampleNum]*3)/4;
  sampleBuffer[sampleNum] = math + delayBuffer[delayNum]/4;
}


void loop() {
  feedback = adc ->analogRead(A3);
  feedback = map(feedback, 0, 232, 0, 475);
  delayRate = adc -> analogRead(A6);
  delayRate = map(delayRate, 0, 232, 1, 10);
  sampleBool = digitalRead(18);
}
