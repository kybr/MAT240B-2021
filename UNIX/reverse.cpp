#include <iostream>
#include <vector>

using namespace std;

#include "everything.h"  // mono

int main(int argc, char* argv[]) {
  // this will not go well for infinite input streams;
  // we assume someone used the 'take.exe' program
  vector<double> input;
  double value;
  while (std::cin >> value)  //
    input.push_back(value);

  //
  //
  while (input.size() > 0) {
    mono(input.back());
    input.pop_back();
  }
}
