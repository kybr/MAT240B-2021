#include "everything.h"

int main(int argc, char* argv[]) {
  const double fC = SAMPLE_RATE * 0.45;
  double phase = 0;

  for (double note = 127; note > 0; note -= 0.001) {
    double f0 = mtof(note);
    double N = fC / f0;
    double v = 0;

    for (int n = 1; n < N; n += 2) {
      double A = pow(-1, (n - 1) / 2) / (n * n);
      v += A * sin(n * phase);
    }

    v *= 8 / (pi * pi);

    mono(v);

    phase += 2 * pi * f0 / SAMPLE_RATE;
    if (phase > 2 * pi) {
      phase -= 2 * pi;
    }
  }
}

/*
int main(int argc, char* argv[]) {
  double t = 0;
  for (int w = 0; w < SAMPLE_RATE; ++w) {
    double f0 = mtof(scale(w, 0, SAMPLE_RATE, 127, 0));
    double N = (SAMPLE_RATE / 2) / f0;

    double v = 0;
    for (int n = 1; n < N; n += 2) {
      double A = pow(-1, (n - 1) / 2) / (n * n);
      double phase = 2 * pi * n * f0 * t;
      v += A * sin(phase);
    }
    v *= 8 / (pi * pi);
    t += 1.0 / SAMPLE_RATE;

    mono(v);
  }
}
*/
