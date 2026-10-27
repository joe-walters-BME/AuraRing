#ifndef AURARINGHR_H
#define AURARINGHR_H
 
#include "Arduino.h"
#include "Filters.h"
#include "MathClassTemplate.h"
 
//The filters are different depending on whehter its PPG or ECG
enum SignalType {PPG, ECG};

class AuraRingHR {
  private:
  // I made 2 different filters to make it easy, 1 for the highpass & lowpass and 1 for the Average Filter
  FiltersTemplate<float> filter;      
  FiltersTemplate<float> avgFilter;
 
  float fs;               // sample rate (Hz)
  int seconds;            //Seconds to average over for average HR
  int minGap;             // fewest samples allowed between two beats
  float peak;             // biggest recent value of the filtered signal
  float prevFiltered;     // filtered value from the previous sample
  long sampleCount;       // total samples seen so far
  long sinceBeat;         // samples since the last beat
  bool haveBeat;          // true once the first beat has been found
  int hrArr[10];          // the once-per-second heart rates
  int arrIndex;           // where the next rate goes in hrArr
  int filled;             // how many rates have been added so far
  long secCount;          // samples since the last once-per-second reading
  float avg;              // latest moving average
  MathClassTemplate<int> math;   // from MathClassTemplate.h
  int ibiArr[120];        // beat intervals (ms) from the current 60 s chunk
  int ddxArr[120];        // differences between successive intervals
  int nIBI;               // how many intervals are stored
  int lastIBI;            // newest beat interval (ms)
  bool newIBI;            // true when BtB just found a new beat interval
  long rmssdCount;        // samples since the 60 s chunk started
  float rmssdVal;         // RMSSD from the last finished chunk (ms)
 
  public:
  float filtered;         // the filtered signal, so the .ino can print it
  int rate;
 
  // type = PPG or ECG, sampleRate in Hz, avgSeconds = averaging intervayl
  AuraRingHR(SignalType type, float sampleRate, int avgSeconds) {
    fs = sampleRate;
    seconds = avgSeconds;
    if (seconds > 10) seconds = 10;
    if (seconds < 1) seconds = 1;
 
    float lowpassHz;
    if (type == ECG) {
      lowpassHz = 40;
      minGap = (int)(0.25 * fs);   // 250 ms
    } else {
      lowpassHz = 5;
      minGap = (int)(0.3 * fs);    // 300 ms
    }
 
    filter.inithighpass(0.5, fs);
    filter.initlowpass(lowpassHz, fs);
    avgFilter.initMovAv(seconds);
 
    peak = 0;
    prevFiltered = 0;
    sampleCount = 0;
    sinceBeat = 0;
    haveBeat = false;
    rate = 0;
    arrIndex = 0;
    filled = 0;
    secCount = 0;
    avg = 0;
    filtered = 0;
    nIBI = 0;
    lastIBI = 0;
    newIBI = false;
    rmssdCount = 0;
    rmssdVal = 0;
  }
 
  //@param sample: The raw signal received by the analog pin. 
  //@return rate: The time elapsed between the most recent signal and the previous time BtB was called
  //@brief: The idea with this function is that it gets called one way or the other for each read. It first determines whether there was a peak, then counts how many samples
  //are taken between spikes. It then uses the sample rate to calculate time between spikes, and then uses that to calculate bpm.
  int BtB(int sample) {
    // clean up the signal
    filtered = filter.lowpass(filter.highpass((float)sample));
    sampleCount++;
    sinceBeat++;
    // Give the filter time to settle
    if (sampleCount < 3 * fs) {
      prevFiltered = filtered;
      return 0;
    }
    // peak is the largest recent value, which slowly decays. Also in this block, when to replace the peak and threshold values
    peak = peak * 0.999;
    if (filtered > peak) peak = filtered;
    float threshold = 0.6 * peak;
    // this big old if statement checks whether the signal is above the threshold, the previous filtered signal
    // basically another sanity check
    if (peak > 10 && filtered > threshold && prevFiltered <= threshold && sinceBeat > minGap) {
      //haveBeat is a Boolean that says 
      if (haveBeat) {
        // count the samples since the last beat and turn that into beats per minute
        float bpm = 60.0 * fs / sinceBeat;
        //This is just a sanity check, making sure the bpm makes sense.
        if (bpm > 30 && bpm < 220) {
          rate = (int)(bpm + 0.5);
          //these two are for the RMSSD, since BtB gets called each sample. It sets lastIBI
          lastIBI = (int)(1000.0 * sinceBeat / fs + 0.5);
          newIBI = true;
        }
      }
      haveBeat = true;
      sinceBeat = 0;
    }
    return rate;
  }
 
//@param sample: The same raw signal
//@return avg: this only returns if there have been enough samples to calculate the average
//@brief: calculates timed average. I'm assuming this will be called along with the BtB function, each 'loop'.
//Once per second it grabs the latest BtB rate, puts it in the array, and adds it to the moving average. 
float TimedAvg(int sample) {
    rate = BtB(sample);
    //Tracks time         
    secCount++;
    if (secCount >= fs) {        
      secCount = 0;
      if (rate > 0) {
        //records the rate
        hrArr[arrIndex] = rate;
        //Increments index
        arrIndex = (arrIndex + 1) % seconds;
        //"Filled" is how many samples are filled.
        if (filled < seconds) 
          filled++;
        avg = avgFilter.MovAv(rate);
      }
    }
    if (filled >= seconds) return avg;
    return 0;
  }
//@param: none; calculates and tracks the value automatically
//@return: Rmssd
//@brief: Calculates RMSSD in 60-second chunks. Has to be called every sample. Returns 0 until the first 60 seconds are done, 
//then the RMSSD (ms) of the last full 60 seconds. 
float RMSSD() {
    rmssdCount++;
    if (newIBI == true) {                                   // BtB just found a new beat interval
      newIBI = false;
      if (nIBI < 120) {                                     // This is where the BtB bit comes in - the ibiArr is set to lastIBI, which is the time b/t beats
        ibiArr[nIBI] = lastIBI;
        nIBI++;
      }
    }
    if (rmssdCount >= 60 * fs) {                    // 60 seconds are up
      if (nIBI >= 2) {
        math.calcDerivative(ibiArr, nIBI, ddxArr);  //this is where the two arrays are differentiated
        rmssdVal = math.rootmeansquare(ddxArr + 1, nIBI - 1);
      }
      rmssdCount = 0;                               // start the next chunk
      nIBI = 0;
    }
    return rmssdVal;
  }
  };

#endif // AURARINGHR_H
 

