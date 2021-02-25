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
  ExpSeg alpha;
  ExpSeg beta;
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
    carrier.zero();
    modulator.zero();
  }
};

struct MyApp : App {
  ControlGUI gui;
  Parameter volume{"/volume", "", -90, "", -90, 0};
  Parameter frequency{"/frequency", "", -30, "", -30, 97};
  ParameterBool randomize{"/randomize", "", 0, "", 0, 1};
  Parameter duration{"/duration", "", 0.1, "", 0.01, 0.6};
  Parameter envelope{"/envelope", "", 0.1, "", 0.0, 1.0};
  Parameter carrier0{"/carrier0", "", 60, "", 0, 127};
  Parameter carrier1{"/carrier1", "", 60, "", 0, 127};
  Parameter modulator0{"/modulator0", "", 60, "", -30, 97};
  Parameter modulator1{"/modulator1", "", 60, "", -30, 97};
  Parameter deviation0{"/deviation0", "", 60, "", 0, 255};
  Parameter deviation1{"/deviation1", "", 60, "", 0, 255};
  Parameter pan0{"/pan0", "", 0.5, "", 0, 1};
  Parameter pan1{"/pan1", "", 0.5, "", 0, 1};

  diy::Timer timer;
  diy::Buffer recorder;
  PolySynth polySynth;

  void onCreate() override {
    polySynth.allocatePolyphony<FrequencyModulationGrain>(2000);
    timer.period(0.8);
    gui.init();
    gui << volume;
    gui << frequency;
    gui << randomize;
    gui << duration;
    gui << envelope;
    gui << carrier0;
    gui << carrier1;
    gui << modulator0;
    gui << modulator1;
    gui << deviation0;
    gui << deviation1;
    gui << pan0;
    gui << pan1;

    // call this after any diy-based entities are created.
    //
    diy::setPlaybackRate(48000);
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

  float lastFrequency{0};

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
      // frequency.hasChange() ???
      if (lastFrequency != frequency.get()) {
        lastFrequency = frequency.get();
        timer.frequency(mtof(frequency.get()));
      }

      if (timer()) {
        auto* voice = polySynth.getVoice<FrequencyModulationGrain>();

        if (randomize.get()) {
          duration.set(clip(rnd::normal() / 10 + 0.2, 0.6, 0.05));
          envelope.set(rnd::uniform(0.9, 0.1));
          carrier0.set(rnd::uniform(127.0));
          carrier1.set(rnd::uniform(127.0));
          modulator0.set(rnd::uniform(97.0, -30.0));
          modulator1.set(rnd::uniform(97.0, -30.0));
          deviation0.set(rnd::uniform(225.0, -30.0));
          deviation1.set(rnd::uniform(225.0, -30.0));
          pan0.set(rnd::uniform());
          pan1.set(rnd::uniform());
        }

        float gain = 0.9;
        float dur = duration.get();
        float env = envelope.get();
        float c0 = mtof(carrier0.get());
        float c1 = mtof(carrier1.get());
        float m0 = mtof(modulator0.get());
        float m1 = mtof(modulator1.get());
        float d0 = mtof(deviation0.get());
        float d1 = mtof(deviation1.get());
        float p0 = pan0.get();
        float p1 = pan1.get();

        printf(
            "duration: %f\n"
            "gain: %f\n"
            "envelope: %f\n"
            "carrier0: %f\n"
            "carrier1: %f\n"
            "modulator0: %f\n"
            "modulator1: %f\n"
            "deviation0: %f\n"
            "deviation1: %f\n"
            "pan0: %f\n"
            "pan1: %f\n",
            dur, gain, env, c0, c1, m0, m1, d0, d1, p0, p1);

        printf("range0: (%f, %f)\n", (c0 - (d0 / m0)), (c0 + (d0 / m0)));
        printf("range1: (%f, %f)\n", (c1 - (d1 / m1)), (c1 + (d1 / m1)));
        printf("====================================================\n");

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
