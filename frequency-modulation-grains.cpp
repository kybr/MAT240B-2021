#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
using namespace al;

#include "synths.h"
using namespace diy;

#include <iostream>
#include <stdexcept>

// FrequencyModulationGrain
//
//
struct FrequencyModulationGrain {
  Sine carrier;
  Sine modulator;
  Line alpha;
  Line beta;
  Line index;
  Line pan;
  AttackDecay envelope;

  diy::FloatPair operator()() {
    if (envelope.decay.done()) return {0.0f, 0.0f};
    modulator.frequency(mtof(beta()));
    carrier.frequency(mtof(alpha()) + index() * modulator());
    float p = pan();
    float v = envelope() * carrier();
    return {(1.0f - p) * v, p * v};
  }

  //
  // - duration: seconds
  // - gain: [0, 1]
  // - attack: [0, 1]
  // - c0, c1, m0, m1: MIDI units
  // - i0, i1: FM index
  // - p0, p1: [0, 1]
  void configure(float duration, float gain, float attack, float c0, float c1,
                 float m0, float m1, float i0, float i1, float p0, float p1) {
    alpha.set(c0, c1, duration);
    beta.set(m0, m1, duration);
    index.set(i0, i1, duration);
    pan.set(p0, p1, duration);
    envelope.set(attack * duration, (1 - attack) * duration, gain);
  }
};

struct MyApp : App {
  ControlGUI gui;
  Parameter volume{"/volume", "", -90, "", -90, 0};

  diy::Timer timer;

  FrequencyModulationGrain g;

  MyApp() {
    g.configure(0.5, 0.9, 0.1, 60.0, 63.0, 3.0, 7.0, 0.0, 10.0, 0.0, 1.0);
    timer.period(0.8);
    //
  }

  void onCreate() override {
    gui.init();
    gui << volume;
  }

  void onAnimate(double dt) override {
    navControl().active(!gui.usingInput());
    //
  }

  void onDraw(Graphics& g) override {
    g.clear(0.25);
    gui.draw(g);
  }

  void onSound(AudioIOData& io) override {
    try {
      while (io()) {
        if (timer()) {
          g.configure(0.5, 0.9, 0.1, rnd::uniform(90.0, 30.0),
                      rnd::normal() * 5 + 60, rnd::uniform(10, -10), 7.0,
                      rnd::uniform(100), rnd::normal() * 10 + 30,
                      rnd::uniform(), rnd::uniform());
        }

        diy::FloatPair p = g();
        float gain = dbtoa(volume.get());
        io.out(0) = p.left * gain;
        io.out(1) = p.right * gain;
      }
    } catch (const std::exception& e) {
      std::cout << "Exception Thrown! " << e.what() << std::endl;
    }
  }
};

int main() {
  MyApp app;
  app.configureAudio(48000, 1024, 2);
  app.start();

  /*
     diy::Timer timer;
  timer.period(0.8);
  FrequencyModulationGrain g;

  while (true) {
    if (timer()) {
      g.configure(0.5, 0.9, 0.1, rnd::uniform(90.0, 30.0),
                  rnd::normal() * 5 + 60, rnd::uniform(10, -10), 7.0,
                  rnd::uniform(100), rnd::normal() * 10 + 30);
    }
    float f = g();
  }
  */
}
