#include "al/core.hpp"
#include "al/util/ui/al_ControlGUI.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_Preset.hpp"
using namespace al;

#include "synths.h"
using namespace diy;

using namespace std;

struct WindowedSync : Edge, Array {
  WindowedSync(unsigned size = 10000) {
    const float pi2 = M_PI * 2;
    resize(size);
    for (unsigned i = 0; i < size; ++i) at(i) = sinf(i * pi2 / size);
  }

  Phasor resonator;

  virtual float operator()() {
    const float index = resonator() * size();
    const float v = get(index) * (1 - phase);
    if (Edge::operator()()) resonator.phase = 0;
    return v;
  }
};

struct MyApp : App {
  ControlGUI gui;
  Parameter frequency{"/frequency", "", 60, "", 0, 127};
  Parameter filter{"/filter", "", 80, "", 0, 127};

  WindowedSync windowedSync;

  void onCreate() override {
    windowedSync.resonator.frequency(mtof(filter));
    windowedSync.frequency(mtof(frequency));
    cout << windowedSync() << endl;
    cout << windowedSync() << endl;
    cout << windowedSync() << endl;
    cout << windowedSync() << endl;
    cout << windowedSync() << endl;
    cout << windowedSync() << endl;
    gui.init();
    gui << frequency << filter;
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
    while (io()) {
      windowedSync.resonator.frequency(mtof(filter));
      windowedSync.frequency(mtof(frequency));
      float f = windowedSync();
      io.out(0) = f;
      io.out(1) = f;
    }
  }
};

int main() {
  MyApp app;
  app.initAudio(SAMPLE_RATE, BLOCK_SIZE, OUTPUT_CHANNELS, INPUT_CHANNELS);
  app.start();
}
