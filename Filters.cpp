#include "Filters.h"
#define PI 3.14159265 // I literally couldn't get PI to work without this. I don't know why. 
Filters::Filters() {
  //way to store the previous outputs/inputs for high and low-pass
  prevLo = 0;
  prevHiOut = 0;
  prevHiIn = 0;
  Window_size = 0;
}
//I stored different high/low fc and fs values for high and low pass
void Filters::initlowpass(const float fc, const float fs) {
  fc_lo = fc;
  fs_lo = fs;
}


float Filters::lowpass(float vin) {
   float tau = 1.0 / (fc_lo * 2 * PI);
  float dt = 1.0 / fs_lo;
  float alpha = dt / (tau + dt);

  float ans = prevLo + alpha * (vin - prevLo);
  //Cataloging (sic?) Previous Low
  prevLo = ans;
  return ans;
}

void Filters::inithighpass(const float fc, const float fs) {
  fc_hi = fc;
  fs_hi = fs;
}

float Filters::highpass(const float vin) {
  //High-pass filter logic given in assignment doc
  float tau = 1.0 / (fc_hi * 2 * PI);
  float dt = 1.0 / fs_hi;
  float alpha = tau / (tau + dt);
  float ans = alpha * (prevHiOut + vin - prevHiIn);
  //Storing previous values for use in next cycle
  prevHiIn = vin;
  prevHiOut = ans;
  return ans;
}

void Filters::initMovAv(int window) { 
  //
  Window_size = window;
}
float Filters::MovAv(uint16_t input) {
  for (uint16_t i = 0; i < Window_size - 1; i++) {
    //Moves each value to the left
    Window_arr[i] = Window_arr[i + 1];
  }
  //Puts the next value (input) at the end
  Window_arr[Window_size-1] = input;
  float total = 0;
  //Sums each term
  for (uint16_t i = 0; i < Window_size; i++) 
    total += Window_arr[i];
  //returns the average
  return total / Window_size;
}
void Filters::initmedian(const uint16_t window) {
  Window_size = window;
}

float Filters::median(const float input) {
  //loads the next term in the same way as MovAv
  for (uint16_t i = 0; i < Window_size - 1; i++) {
    Window_arr[i] = Window_arr[i + 1];
  }
  Window_arr[Window_size-1] = input;
  float dummy[Window_size] {0};
  bool swapped;
  //Loading a dummy array so as not to touch the original
  for (int i = 0; i < Window_size; i++) {
    dummy[i] = Window_arr[i];
    }
    //bubble sorting algorithm; checks each number against each number after it), and swaps them if necessary
    for (int i = 0; i < Window_size; i++) {
        swapped = false;
        for (int i = 0; i < Window_size; i++) {
            if (dummy[i] < dummy[i+1]) {
                float temp = dummy[i];
                dummy[i]=dummy[i+1];
                dummy[i+1] = temp;
                swapped = true;
            }
        }
        if (!swapped)
        break;
    }
    //returns either the middle number or the avg of the 2 middle numbers
    if (Window_size % 2 == 1)
         return float(dummy[(Window_size-1)/2]);
    else
         return float(dummy[Window_size/2] + dummy[(Window_size - 2)/2])/ 2;
}

void Filters::bodeplot(const FilterType f, float mag[], float freq[], const uint16_t samples, bool debug) {
  //checks if the filter type is median, terminates if so
  if (f != LOWPASS && f != HIGHPASS) {
    Serial.println("bodeplot: only LOWPASS and HIGHPASS are supported");
    return;
  }
  //assigns fc depending on filtertype
  float fc;
  if (f == LOWPASS) {
    fc = fc_lo;
  } else {
    fc = fc_hi;
  }

  //Frequencies tested (simulating a sweep)
  const float testFreq[5] = {0.5, 1, 10, 100, 1000};   // Hz
  const float saveFsLo = fs_lo; 
  const float saveFsHi = fs_hi;

  if (debug) Serial.println("Freq (Hz), Magnitude (dB)");

  for (uint16_t i = 0; i < samples && i < 5; i++) {
    freq[i] = testFreq[i];

    // Simulate each freq at 50 samples per cycle (avoids issue at low f)
    float simFs = 50 * freq[i];
    fs_lo = simFs;
    fs_hi = simFs;
    //resetting previous values to start each sweep from 0
    prevLo = 0;                
    prevHiIn = 0;
    prevHiOut = 0;

    float tau = 1 / (2 * PI * fc);
    // letting filter settle
    double settle = 10 * tau * simFs;     
    double total  = settle + 70;         

    //Max and min values
    float inPk = 0;
    float outPk = 0;
    for (int n = 0; n < total; n++) {
      // Sine wave
      float x = sin(2 * PI * freq[i] * n / simFs);   
      float y;
      if (f == LOWPASS) {
        y = lowpass(x);
      } else {
        y = highpass(x);
      }
      if (n >= settle) {
        //Determining max and min values                             
        if (x > inPk)  inPk  = x;
        if (y > outPk) outPk = y;
      }
    }
    //magnitude
    mag[i] = 20 * log10(outPk / inPk);           

    if (debug == true) {
      Serial.print(freq[i]);
      Serial.print(", ");
      Serial.println(mag[i]);
    }
  }
  // put the real sample rates back
  fs_lo = saveFsLo;              
  fs_hi = saveFsHi;
  //reset the filter
  prevLo = 0;                    
  prevHiIn = 0;
  prevHiOut = 0;
}
