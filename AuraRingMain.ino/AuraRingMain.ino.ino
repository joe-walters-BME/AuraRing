//File name: AuraRingMain.ino
//Author: Joe Walters
//6+2: waltejp
//Email: waltejp@mail.uc.edu
//Class: BME 5102/6002
//Date: 10/28/26
//Link to GitHub Repository: https://github.com/joe-walters-BME/AuraRing.git
//Honor statement: I certify that I have neither given nor received
// unauthorized aid on this assignment and will not
// share my code with anyone after this semester. I
// certify that I did not use ChatGPT, BearcatGPT
// Gemini, JetBrains AI, or any other AI, LLM, etc.
// to complete any part of this assignment.
//
//Description: This one just initalizes the classes, sets the parameters and prints the output
#include "AuraRingHR.h"

const float FS = 100;         
const int AVG_SECONDS = 10;     
 


AuraRingHR heart(PPG, FS, AVG_SECONDS);

void setup() {
  Serial.begin(9600);
}

void loop() {
  int raw = analogRead(A0);
  float avg = heart.TimedAvg(raw);   
  float rmssd = heart.RMSSD();
  Serial.print("Raw:");
  Serial.print(raw);
  Serial.print(",");
  Serial.print("Filtered:");
  Serial.print(heart.filtered);
  Serial.print(",");
  Serial.print("Average:");
  Serial.print(avg);
  Serial.print(",");
  Serial.print("RMSSD:");
  Serial.println(rmssd);

  delay(1000 / FS);                  
}