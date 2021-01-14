#include "everything.h"

int main(int argc, char* argv[]) {
  float phase = 0;
  for (float note = 127; note > 0; note -= 0.001) {
    float frequency = mtof(note);
    float v = sin(phase);
    mono(v * 0.707);
    phase += 2 * pi * frequency / SAMPLE_RATE;
    if (phase > 2 * pi)  //
      phase -= 2 * pi;
  }
}
