#include "everything.h"

int main(int argc, char* argv[]) {
  const double fC = SAMPLE_RATE * 0.45;
  double phase = 0;

  for (double note = 127; note > 0; note -= 0.001) {
    double f0 = mtof(note);
    double N = fC / f0;
    double v = 0;

    for (int n = 1; n < N; n += 2) {
      v += sin(n * phase) / n;
    }

    v *= 3 / pi;

    mono(v);

    phase += 2 * pi * f0 / SAMPLE_RATE;
    if (phase > 2 * pi) {
      phase -= 2 * pi;
    }
  }
}
