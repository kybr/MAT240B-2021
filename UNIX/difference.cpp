#include <iostream>

#include "everything.h"  // mono

int main(int argc, char* argv[]) {
  double previous = 0;
  double value;
  while (std::cin >> value) {
    mono(value - previous);
    previous = value;
  }
}
