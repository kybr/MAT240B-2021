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

  void onProcess(AudioIOData& io) override {
    while (io()) {
      diy::FloatPair pair = operator()();
      // Always remember to add to the audio output!!
      // Otherwise you will be overwriting everyone else!!
      io.out(0) += pair.left;
      io.out(1) += pair.right;

      if (envelope.decay.done()) {
        free();
        break;
      }
    }
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
        float attack = clip(rnd::uniform(0.5, 0.1));
        float c0 = rnd::uniform(127.0f);
        float c1 = clip(rnd::normal() * 5 + 60, 127.0f);
        float m0 = rnd::uniform(127.0f);
        float m1 = 7;
        float i0 = rnd::uniform(100);
        float i1 = rnd::normal() * 10 + 30;
        float p0 = rnd::uniform();
        float p1 = rnd::uniform();
        voice->configure(dur, gain, attack, c0, c1, m0, m1, i0, i1, p0, p1);

        polySynth.triggerOn(voice);
      }

      float gain = dbtoa(volume.get());
      io.out(0) *= gain;
      io.out(1) *= gain;

      recorder(0.5f * (io.out(0) + io.out(1)));
    }
  }
};

int main() {
  MyApp app;
  app.configureAudio(48000, 1024, 2);
  app.start();
}
