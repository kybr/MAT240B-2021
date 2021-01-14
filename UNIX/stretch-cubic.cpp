#include <iostream>
#include <vector>

using namespace std;

#include "everything.h"  // mono

int main(int argc, char* argv[]) {
  double factor = 2.0;
  if (argc > 1) {
    factor = std::stof(argv[1]);

    if (factor <= 1) {
      fprintf(stderr, "choose a number >1\n");
      return 1;
    }
  }

  vector<double> input;
  double value;
  while (std::cin >> value)  //
    input.push_back(value);

  vector<double> output;

  //
  // XXX put your code here. make the output `factor` times longer than the
  // input. use cubic interpolation to generate new sample values.
  //
  // see http://paulbourke.net/miscellaneous/interpolation/
  //

  for (double v : output) {
    mono(v);
  }
}
