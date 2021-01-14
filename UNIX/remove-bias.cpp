#include <iostream>
#include <vector>

#include "everything.h"  // mono

int main(int argc, char* argv[]) {
  // this will not go well for infinite input streams;
  // we assume someone used the 'take.exe' program
  std::vector<double> input;

  double sum = 0;

  double value;
  while (std::cin >> value) {
    sum += value;
    input.push_back(value);
  }

  double mean = sum / input.size();

  for (double v : input) {
    mono(v - mean);
  }
}
