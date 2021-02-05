#include "Gamma/SoundFile.h"
using namespace gam;

#include "al/app/al_App.hpp"
using namespace al;

#include "synths.h"  // PluckedString, Edge
using namespace diy;

struct MyApp : App {
  Edge edge;
  PluckedString string;

  void onCreate() override {
    float d = rnd::uniform(0.5, 7.0);
    float f = mtof(rnd::uniform(21, 60));
    string.set(f, d);
    string.pluck();
    edge.period(d);
  }

  Array recording;
  void onSound(AudioIOData& io) override {
    while (io()) {
      if (edge()) {
        float d = rnd::uniform(0.5, 7.0);
        float f = mtof(rnd::uniform(21, 60));
        string.set(f, d);
        string.pluck();
        edge.period(d);
      }
      float f = string();
      recording(f);
      io.out(0) = f;
      io.out(1) = f;
    }
  }

  bool onKeyDown(const Keyboard& k) override {
    recording.save("karplus-strong-recording.wav");
    return true;
  }
};

int main() {
  MyApp app;
  app.configureAudio(SAMPLE_RATE, BLOCK_SIZE, OUTPUT_CHANNELS, INPUT_CHANNELS);
  app.start();
}
