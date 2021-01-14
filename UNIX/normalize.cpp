#include <iostream>
#include <vector>

#include "everything.h"  // mono

int main(int argc, char* argv[]) {
  // this will not go well for infinite input streams;
  // we assume someone used the 'take.exe' program
  std::vector<double> input;

  double maximum = 0;

  double value;
  while (std::cin >> value) {
    if (abs(value) > maximum) {
      maximum = abs(value);
    }
    input.push_back(value);
  }

  for (double v : input) {
    mono(v / maximum);
  }

  fprintf(stderr, "v / %lf\n", maximum);
}
