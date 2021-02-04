#include "Gamma/SoundFile.h"
using namespace gam;

#include "al/core.hpp"
using namespace al;

#include "synths.h"
using namespace diy;

// https://en.wikipedia.org/wiki/Harmonic_oscillator
struct System {
  // this the whole state of the simulation
  //
  float position{0};
  float velocity{0};

  float operator()() {
    // semi-implicit Euler integration with time-step 1 but the sample rate is
    // "baked into" the constants. string force and damping force are
    // accumulated into velocity. mass is 1, so it disappears.
    //
    float acceleration = 0;
    acceleration += -springConstant * position;  // resting length is 0
    acceleration += -dampingCoefficient * velocity;

    velocity += acceleration;
    position += velocity;
    return position;
  }

  void drive(float position_of_other_spring) {
    //
  }
  // "kick" the mass-spring system such that we get a nice (-1, 1) oscillation.
  //
  void trigger() {
    // we want the "mass" to move in (-1, 1). what is the potential energy of a
    // mass-spring system at 1? PE == k * x * x / 2 == k / 2. so, we want a
    // system with k / 2 energy, but we don't want to just set the displacement
    // to 1 because that would make a click. instead, we want to set the
    // velocity. what velocity would we need to have energy k / 2? KE == m * v *
    // v / 2 == k / 2. or v * v == k. so...
    //
    velocity += sqrt(springConstant);
    // consider triggering at a level depending on frequency according to the
    // Fletcher-Munson curves.
  }

  float ke() { return velocity * velocity / 2; }
  float pe() { return position * position * springConstant / 2; }
  float te() { return ke() + pe(); }

  float springConstant{0};      // N/m
  float dampingCoefficient{0};  // N·s/m
  void recalculate() {
    // sample rate is "baked into" these constants to save on per-sample
    // operations.
    dampingCoefficient = 2 / (_decayTime * SAMPLE_RATE);
    springConstant = pow(_frequency * M_2PI / SAMPLE_RATE, 2) +
                     1 / pow(_decayTime * SAMPLE_RATE, 2);
  }

  // we keep these around so that we can set each independently
  //
  float _frequency{0};  // Hertz
  float _decayTime{0};  // seconds

  void set(float hertz, float seconds) {
    _frequency = hertz;
    _decayTime = seconds;
    recalculate();
  }
  void frequency(float hertz) {
    _frequency = hertz;
    recalculate();
  }
  void decayTime(float seconds) {
    // https://www.dsprelated.com/freebooks/mdft/Audio_Decay_Time_T60.html
    _decayTime = seconds;
    recalculate();
  }
};

struct MyApp : App {
  Edge edge;
  System a, b, c;
  void onCreate() override {
    a.set(180, 0.15);
    b.set(333, 0.15);
    c.set(567, 0.15);
    a.trigger();
    edge.period(1);
    //
  }

  Array recording;

  void onSound(AudioIOData& io) override {
    while (io()) {
      if (edge()) {
        // float f = mtof(rnd::uniform(27, 50));
        // float t = rnd::uniform(0.005, 1.7);
        // edge.period(t);
        // system.set(f, t / 3);
        // system.trigger();

        // a.trigger();
        // b.trigger();
        // c.trigger();
      }
      if (a.te() < 0.000328986 / 3000) a.trigger();
      if (b.te() < 0.000328986 / 3000) b.trigger();
      if (c.te() < 0.000328986 / 3000) c.trigger();
      a.velocity += b.te() / 20;
      b.velocity += c.te() / 20;
      c.velocity += a.te() / 20;
      float f = a() * (b() + c());
      f = sigmoid_bipolar(f / 3);
      recording(f);
      io.out(0) = f;
      io.out(1) = f;
    }
  }
  void onKeyDown(const Keyboard&) override { recording.save("wtf.wav"); }
};

int main() {
  MyApp app;
  app.initAudio(SAMPLE_RATE, BLOCK_SIZE, OUTPUT_CHANNELS, INPUT_CHANNELS);
  app.start();
}