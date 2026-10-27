#include "Arduino.h"
#include "ttestTable.h"
template<typename Type>
class MathClassTemplate {
  public:
  Type sum(const Type* arr, int size);
  Type average(const Type* arr, int size);
  Type stdev(const Type* arr, int size, int PoS = 1);
  Type variance(const Type* arr, int size, int PoS);
  Type std_error(const Type* arr, int size);
  Type median(const Type* arr, int size);
  Type MAX(const Type* arr, int size);
  Type MIN(const Type* arr, int size);
  int Max_index(const Type* arr, int size);
  int Min_index(const Type* arr, int size);
  int spec_index(const Type* arr, int size, Type val);
  Type peak_2_peak(const Type* arr, int size);
  void calcDerivative(const Type arr[], const int samples, Type ddx[]);
  Type  calculateIntegral(const Type arr[], const int samples, float time_step);
  Type rootmeansquare(const Type arr[], const int samples);
  bool ttest(const Type arr1[], const Type arr2[], const int size1, const int size2);
};
 
//Calls the sum function, returns sum/size
template<typename Type>
Type MathClassTemplate<Type>::average(const Type* arr, int size) {
    double total = sum(arr, size);
    return total / size;
}
 
//First calculates the mean, then goes through and calculates the sum of differences squared, then
//takes the square root of the SOS divided by either size or size-1
template<typename Type>
Type MathClassTemplate<Type>::stdev(const Type* arr, int size, int PoS) {
    double sos = 0;
    Type st_dev;
    double mean = average(arr, size);
    int type;
    for (int i = 0; i < size; i++) {
        sos +=( (arr[i] - mean) * (arr[i] - mean));
    }
    if (PoS == 0)
        st_dev = sqrt(sos / size);
    else
        st_dev = sqrt(sos / (size-1));
 
    return st_dev;
}
//Same thing as st.dev, but doesn't take the square root
template<typename Type>
Type MathClassTemplate<Type>::variance(const Type* arr, int size, int PoS) {
    double sos = 0;
    Type variance = 0;
    double mean = average(arr, size);
    int type;
    for (int i = 0; i < size; i++) {
        sos +=( (arr[i] - mean) * (arr[i] - mean));
    }
    if (PoS == 0)
        variance = (sos / size);
    else
        variance = (sos / (size-1));
    return variance;
}
 
//sums the array
template<typename Type>
Type MathClassTemplate<Type>::sum(const Type* arr, int size) {
    Type total = 0;
    for (int i = 0; i < size; i++) {
        total += arr[i];
    }
    return total;
}
//calculates the standard error of the array
template<typename Type>
Type MathClassTemplate<Type>::std_error(const Type* arr, int size) {
    double st_dev = stdev(arr, size);
    return st_dev/sqrt(size);
}
template<typename Type>
Type MathClassTemplate<Type>::median(const Type* arr, int size) {
    Type dummy[size];
    bool swapped;
    //Loading a dummy array so as not to touch the original
    for (int i = 0; i < size; i++) {
        dummy[i] = arr[i];
    }
    //bubble sorting algorithm; checks each number against each number after it), and swaps them if necessary
    for (int i = 0; i < size; i++) {
        swapped = false;
        for (int i = 0; i < size; i++) {
            if (dummy[i] < dummy[i+1]) {
                swap(dummy[i], dummy[i+1]);
                swapped = true;
            }
        }
        if (!swapped)
        break;
    }
    //returns either the middle number or the avg of the 2 middle numbers
    if (size % 2 == 1)
         return dummy[(size-1)/2];
    else
         return double(dummy[size/2] + dummy[(size - 2)/2])/ 2;
    }
//iterates through the array, replacing the max value if the checked value is higher.
template<typename Type>
Type MathClassTemplate<Type>::MAX(const Type* arr, int size) {
    Type Maximum = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > Maximum)
            Maximum = arr[i];
    }
    return Maximum;
}
//iterates through the array, replacing the min value if the checked value is lower.
template<typename Type>
Type MathClassTemplate<Type>::MIN(const Type* arr, int size) {
    Type Minimum = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] < Minimum)
            Minimum = arr[i];
    }
    return Minimum;
}
//iterates through the array, replacing the index value if the checked value is higher than the max.
template<typename Type>
int MathClassTemplate<Type>::Max_index(const Type* arr, int size) {
  int index = 0;
    Type Maximum = arr[0];
   for (int i = 1; i < size; i++) {
         if (arr[i] > Maximum)
            index = i;
    }
    return index;
}
 
//iterates through the array, replacing the index value if the checked value is lower than the min.
template<typename Type>
int MathClassTemplate<Type>::Min_index(const Type* arr, int size) {
    int index = 0;
    Type Minimum = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] < Minimum)
            index = i;
    }
    return index;
}
template<typename Type>
int MathClassTemplate<Type>::spec_index(const Type* arr, int size, Type val) {
    int index = 0;
    bool FOUND = false;
    //checks each term individually, saving [i] if the values match
    for (int i = 0; i < size; i++) {
        if (arr[i] == val)
        index = i;
        FOUND = true;
    }
    //FOUND != true only if the value is not in the array
    if (FOUND!=true) {
        index = -1;
    }
    return index;
}
template<typename Type>
Type MathClassTemplate<Type>::peak_2_peak(const Type* arr, int size) {
    //Max-Min
    return MAX(arr, size) - MIN(arr, size);
}
template<typename Type>
void MathClassTemplate<Type>::calcDerivative(const Type arr[], const int samples, Type ddx[]){
    //passes by reference an array, into which populates the difference between each term and the next
    ddx[0]=0;
    for (int i = 1; i < samples; i++) {
        ddx[i]=arr[i]-arr[i-1];
    }
}
template<typename Type>
Type MathClassTemplate<Type>::calculateIntegral(const Type arr[], const int samples, float time_step) {
    double sum = 0;
    //goes through each term, making a 'rectangle' for it and summing them up.
    for (int i = 1; i < samples; i++) {
        sum += (arr[i])*time_step;
    }
    return sum;
}
 
template<typename Type>
Type MathClassTemplate<Type>::rootmeansquare(const Type arr[], const int samples) {
    double sos = 0;
    //squaring every term and summing them
    for (int i = 0; i < samples; i++) {
        sos += (double)(arr[i])*arr[i];
    }
    return sqrt(sos/samples);
}
 
template<typename Type>
bool MathClassTemplate<Type>::ttest(const Type arr1[], const Type arr2[], const int size1, const int size2) {
    //return boolean; true of significantly different, false if not
    bool signif;
    //creating an array to load differences into
    double diffs[size1] = {0};
    int DOF = size1-1;
    double difftotal = 0;
    //populating diffs[] array with the differences for each pair, as well as summing the total difference
    for (int i = 0; i < size1; i++) {
        diffs[i] = (double)arr2[i] - (double)arr1[i];
        difftotal += diffs[i];
    }
    //average difference
    double diffmean = difftotal / (size1);
    double diff_sos = 0;
    //calculating the sum of squares of the differences
    for (int i = 0; i < size1; i++) {
         diff_sos += (diffs[i] - diffmean) * (diffs[i] - diffmean);
    }
    //basic calculations, just math
    double diffstd = sqrt(diff_sos / (size1-1));
    double SE_d = diffstd/sqrt(size1);
    double t_stat = diffmean/SE_d;
    //checking whether T is greater than T
    if (t_stat > alpha95[DOF])
        signif = true;
    else
        signif = false;
    return signif;
}
