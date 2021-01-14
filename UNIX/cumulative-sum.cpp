#include <iostream>

#include "everything.h"  // mono

int main(int argc, char* argv[]) {
  double sum = 0;
  double value;
  while (std::cin >> value) {
    sum += value;
    mono(sum);
  }
}
