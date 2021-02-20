// Karl Yerkes / 2021-02-18 / MAT240B
//
// Frequency Modulation Granular Synthesizer
//
//

#include "al/app/al_App.hpp"
#include "al/math/al_Functions.hpp"  // al::clip
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
using namespace al;

#include "synths.h"
using namespace diy;

// FrequencyModulationGrain
//
struct FrequencyModulationGrain : SynthVoice {
  Sine carrier;
  Sine modulator;
  ExpSeg alpha;  // we use ExpSeg for exponential segments
  ExpSeg beta;   // these are good for frequency envelopes in Hertz
  Line index;
  Line pan;
  AttackDecay envelope;

  void onProcess(AudioIOData& io) override {
    while (io()) {
      modulator.frequency(beta());
      carrier.frequency(alpha() + index() * modulator());
      float p = pan();
      float v = envelope() * carrier();
      io.out(0) += (1 - p) * v;
      io.out(1) += p * v;

      if (envelope.decay.done()) {
        free();
        break;
      }
    }
  }

  //
  // - duration: seconds
  // - gain: [0, 1]
  // - env: [0, 1]
  // - c0, c1, m0, m1: MIDI units
  // - i0, i1: FM index
  // - p0, p1: [0, 1]
  void set(float dur, float gain, float env, float c0, float c1, float m0,
           float m1, float i0, float i1, float p0, float p1) {
    alpha.set(c0, c1, dur);
    beta.set(m0, m1, dur);
    index.set(i0, i1, dur);
    pan.set(p0, p1, dur);
    envelope.set(env * dur, (1 - env) * dur, gain);
  }
};

struct MyApp : App {
  ControlGUI gui;
  Parameter volume{"/volume", "", -90, "", -90, 0};

  diy::Timer timer;
  diy::Buffer recorder;
  PolySynth polySynth;

  void onCreate() override {
    polySynth.allocatePolyphony<FrequencyModulationGrain>(2000);
    timer.period(0.2);
    gui.init();
    gui << volume;
  }
  void onExit() override { recorder.save("fm-grains.wav"); }

  void onAnimate(double dt) override {
    navControl().active(!gui.usingInput());
    //
  }

  void onDraw(Graphics& g) override {
    g.clear(0.25);
    gui.draw(g);
  }

  void onSound(AudioIOData& io) override {
    // render all active synth voices (grains) into the output buffer
    //
    polySynth.render(io);

    // reset the frame so we can go over the frame again below
    //
    io.frame(0);

    // trigger new voices on the timer and apply global volume
    //
    while (io()) {
      if (timer()) {
        auto* voice = polySynth.getVoice<FrequencyModulationGrain>();

        float dur = clip(rnd::normal() / 10 + 0.2, 1.0, 0.01);
        float gain = 0.9;
        float env = clip(rnd::uniform(0.5, 0.1));
        float c0 = mtof(rnd::uniform(127.0f));
        float c1 = mtof(clip(rnd::normal() * 5 + 60, 127.0f));
        float m0 = mtof(rnd::uniform(127.0f));
        float m1 = mtof(7);
        float d0 = rnd::uniform(5.0, 0.1);
        float d1 = (rnd::normal() * 10 + 30);
        float p0 = rnd::uniform();
        float p1 = rnd::uniform();
        voice->set(dur, gain, env, c0, c1, m0, m1, d0 / m0, d1 / m1, p0, p1);

        polySynth.triggerOn(voice);
      }

      recorder(0.5f * (io.out(0) + io.out(1)));

      float gain = dbtoa(volume.get());
      io.out(0) *= gain;
      io.out(1) *= gain;
    }
  }
};

int main() {
  MyApp app;
  app.configureAudio(48000, 1024, 2);
  app.start();
}
