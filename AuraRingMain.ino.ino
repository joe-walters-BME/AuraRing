//File name: AuraRingMain.ino
//Author: Joe Walters
//6+2: waltejp
//Email: waltejp@mail.uc.edu
//Class: BME 5102/6002
//Date: 10/14/26
//Link to GitHub Repository: https://github.com/joe-walters-BME/AuraRing.git
//Honor statement: I certify that I have neither given nor received
// unauthorized aid on this assignment and will not
// share my code with anyone after this semester. I
// certify that I did not use ChatGPT, BearcatGPT
// Gemini, JetBrains AI, or any other AI, LLM, etc.
// to complete any part of this assignment.
//
//Description: This one just initalizes and calls all the different filters on the ppg data. Most of the commentary is on the .cpp file of the filters class

#include "Filters.h"
#include "SampleData.h"

Filters filter1;
Filters filter2;
Filters filter3;
Filters filter4;
const float fc = 4;
const float fs = 100;
const uint16_t N = 30;
float mag[N], freq[N];

void setup() {
  Serial.begin(9600);

  filter1.initlowpass(4, 100.0);    // fc, fs
  filter2.inithighpass(4.0, 100.0);
  filter3.initMovAv(4);
  filter4.initmedian(4);


  // debug is left off, so nothing is printed by bodeplot itself.
  // The results come back through the arrays.
  filter1.bodeplot(LOWPASS, mag, freq, N, false);

  Serial.println("LOWPASS");
  for (uint16_t i = 0; i < N; i++) {
    Serial.print(freq[i]);
    Serial.print(" Hz: ");
    Serial.print(mag[i]);
    Serial.println(" dB");
  }
  }

void loop() {
  for (int i = 0; 1 < ppg1.samples; i++){
    Serial.print("Raw:");
    Serial.print(ppg1.data[i]);
    Serial.print(",LowPass:");
    Serial.print(filter1.lowpass(ppg1.data[i]));
    Serial.print(",High Pass:");
    Serial.print(filter2.highpass(ppg1.data[i]));
    Serial.print(",Moving Avg:");
    Serial.print(filter3.MovAv(ppg1.data[i]));
    Serial.print(",Median:");
    Serial.println(filter4.median(ppg1.data[i]));
    delay(200);
  }
}
