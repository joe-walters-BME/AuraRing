#ifndef FILTERS_H
#define FILTERS_H
#include "Arduino.h"
enum FilterType { LOWPASS, HIGHPASS, MEDIAN, MOVAV };
class Filters{
  private:
  float fs_lo;
  float fc_lo;
  float fs_hi;
  float fc_hi;
  float prevLo, prevHiOut, prevHiIn;
  int Window_size;
  float Window_arr[10]={0};
  float freq;

  public:
  Filters();
  void initlowpass(const float fc, const float fs);
  float lowpass(float input);
  void inithighpass(const float fc, const float fs);
  float highpass(float input);
  void initMovAv(int window);
  float MovAv(uint16_t input);
  void initmedian(const uint16_t window);
  float median(const float input);
  void bodeplot(const FilterType f, float mag[], float freq[], const uint16_t samples, bool debug);
};

#endif // AURARING_FILTERS_H