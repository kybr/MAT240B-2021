#include "everything.h"

int main(int argc, char* argv[]) {
  double f0 = 800;
  double N = (SAMPLE_RATE / 2) / f0;
  double t = 0;

  for (int w = 0; w < SAMPLE_RATE; ++w) {
    double v = 0;

    for (int n = 1; n < N; n += 2) {
      double A = pow(-1, (n - 1) / 2) / (n * n);
      double phase = 2 * pi * n * f0 * t;
      // double phase = fmod(2 * pi * n * f0 * t, 2 * pi);
      v += A * sin(phase);
    }
    v *= 8 / (pi * pi);
    t += 1.0 / SAMPLE_RATE;

    mono(v);
  }

  fprintf(stderr, "N:%lf\n", N);
}
