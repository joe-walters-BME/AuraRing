#ifndef FILTERSTEMPLATE_H
#define FILTERSTEMPLATE_H
#include "Arduino.h"
enum FilterType { LOWPASS, HIGHPASS, MEDIAN, MOVAV };
template<typename Type>
class FiltersTemplate {
  private:
  float fs_lo;
  float fc_lo;
  float tau_lo;
  float dt_lo;
  float alpha_lo;
  float fs_hi;
  float fc_hi;
  float tau_hi;
  float dt_hi;
  float alpha_hi;
  float prevLo, prevHiOut, prevHiIn;
  uint16_t Window_size;
  Type Window_arr[10];
 
  public:
  FiltersTemplate();
  void initlowpass(const float fc, const float fs);
  Type lowpass(Type input);
  void inithighpass(const float fc, const float fs);
  Type highpass(Type input);
  void initMovAv(uint16_t window);
  float MovAv(Type input);
  void initmedian(const uint16_t window);
  Type median(Type input);
  void bodeplot(const FilterType f, float mag[], float freq[], const uint16_t samples, bool debug);
  };
template<typename Type>
FiltersTemplate<Type>::FiltersTemplate() {
  //way to store the previous outputs/inputs for high and low-pass
  prevLo = 0;
  prevHiOut = 0;
  prevHiIn = 0;
  alpha_lo = 0;
  alpha_hi = 0;
  Window_size = 0;
  for (uint16_t i = 0; i < 10; i++) Window_arr[i] = 0;
}
//I stored different high/low fc and fs values for high and low pass
template<typename Type>
void FiltersTemplate<Type>::initlowpass(const float fc, const float fs) {
  fc_lo = fc;
  fs_lo = fs;
  tau_lo = 1.0 / (fc_lo * 2 * PI);
  dt_lo = 1.0 / fs_lo;
  alpha_lo = dt_lo/(tau_lo+dt_lo);
}
 
template<typename Type>
Type FiltersTemplate<Type>::lowpass(Type vin) {
  float ans = prevLo + alpha_lo * (vin - prevLo);
  //Cataloging (sic?) Previous Low
  prevLo = ans;
  return (Type)ans;
}
template<typename Type>
void FiltersTemplate<Type>::inithighpass(const float fc, const float fs) {
  fc_hi = fc;
  fs_hi = fs;
  tau_hi = 1.0 / (fc_hi * 2 * PI);
  dt_hi = 1.0 / fs_hi;
  alpha_hi = tau_hi / (tau_hi + dt_hi);
}
template<typename Type>
Type FiltersTemplate<Type>::highpass(Type vin) {
  //High-pass filter logic given in assignment doc
 
  float ans = alpha_hi * (prevHiOut + vin - prevHiIn);
  //Storing previous values for use in next cycle
  prevHiIn = vin;
  prevHiOut = ans;
  return (Type)ans;
}
template<typename Type>
void FiltersTemplate<Type>::initMovAv(uint16_t window) {
  Window_size = (window > 10) ? 10 : window;
}
template<typename Type>
float FiltersTemplate<Type>::MovAv(Type input) {
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
template<typename Type>
void FiltersTemplate<Type>::initmedian(const uint16_t window) {
  Window_size = (window > 10) ? 10 : window;
}
 
template<typename Type>
Type FiltersTemplate<Type>::median(Type input) {
  //loads the next term in the same way as MovAv
  for (uint16_t i = 0; i < Window_size - 1; i++) {
    Window_arr[i] = Window_arr[i + 1];
  }
  Window_arr[Window_size-1] = input;
  Type dummy[10] = {0};
  //Loading a dummy array so as not to touch the original
  for (uint16_t i = 0; i < Window_size; i++) {
    dummy[i] = Window_arr[i];
  }
  //bubble sorting algorithm; swaps neighbours until sorted
  for (uint16_t pass = 0; pass < Window_size - 1; pass++) {
    bool swapped = false;
    for (uint16_t j = 0; j < Window_size - 1 - pass; j++) {
      if (dummy[j] > dummy[j+1]) {
        Type temp = dummy[j];
        dummy[j] = dummy[j+1];
        dummy[j+1] = temp;
        swapped = true;
      }
    }
    if (!swapped)
      break;
  }
  //returns either the middle number or the avg of the 2 middle numbers
  if (Window_size % 2 == 1)
    return dummy[(Window_size-1)/2];
  else
    return (Type)((float(dummy[Window_size/2]) + float(dummy[(Window_size - 2)/2])) / 2);
}
 
template<typename Type>
void FiltersTemplate<Type>::bodeplot(const FilterType f, float mag[], float freq[], const uint16_t samples, bool debug) {
  //checks if the filter type is median, terminates if so
  if (f != LOWPASS && f != HIGHPASS) {
    Serial.println("bodeplot: only LOWPASS and HIGHPASS are supported");
    return;
  }
  float fs;
  float dt;
  float tau;
  //assigns fc depending on filtertype
  float fc;
  if (f == LOWPASS) {
    fc = fc_lo;
    fs = fs_lo;
    dt = dt_lo;
    tau = tau_lo;
  } else {
    fc = fc_hi;
    fs = fs_hi;
    dt = dt_hi;
    tau = tau_hi;
  }
 
  //Frequencies tested (simulating a sweep)
  const float testFreq[5] = {0.5, 1, 10, 100, 1000};   // Hz
 
 
  if (debug) Serial.println("Freq (Hz), Magnitude (dB)");
 
  for (uint16_t i = 0; i < samples && i < 5; i++) {
    freq[i] = testFreq[i];
 
    //resetting previous values (if the filter has been called before) to start each sweep from 0
    prevLo = 0;
    prevHiIn = 0;
    prevHiOut = 0;
 
    // letting filter settle
    unsigned long settle = 10 * tau * fs;
    unsigned long total = settle + fs / freq[i] + 20;
    //Max and min values
    float inPk = 0;
    float outPk = 0;
    for (unsigned long n = 0; n < total; n++) {
      // Sine wave
      float x = sin(2 * PI * freq[i] * n / fs);
      float y;
      if (f == LOWPASS) {
        y = lowpass(x);
      } else {
        y = highpass(x);
      }
      if (n >= settle) {
      //Determining max and min values
      if (fabs(x) > inPk)  inPk  = fabs(x);
      if (fabs(y) > outPk) outPk = fabs(y);
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
  //reset the filter
  prevLo = 0;
  prevHiIn = 0;
  prevHiOut = 0;
}

 
#endif // FILTERSTEMPLATE_H
