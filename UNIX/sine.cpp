#include <string>

#include "everything.h"  // mono

int main(int argc, char* argv[]) {
  double frequency = 440.0;
  if (argc > 1) {
    frequency = std::stof(argv[1]);
  }

  double duration = 1;
  if (argc > 2) {
    duration = std::stof(argv[2]);
  }

  double phase = 0;
  for (int i = 0; i < duration * SAMPLE_RATE; ++i) {
    double v = sin(phase);
    mono(v);

    phase += 2 * pi * frequency / SAMPLE_RATE;

    // Wrap phase into the range (0, 2𝞹). If we didn't, phase would get large.
    // As valid numbers are not uniformly distributed across the range of
    // floating point numbers, we may get a sort of rounding error.
    //
    if (phase > 2 * pi) {
      phase -= 2 * pi;
    }
  }

  return 0;
}
