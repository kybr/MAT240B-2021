#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
using namespace al;

#include "synths.h"
using namespace diy;

struct MyApp : App {
  ControlGUI gui;
  ParameterInt active{"/active", "", 0, "", 0, 1000};
  Parameter value{"/value", "", 0, "", -1, 1};
  Phasor phasor;

  bool shouldPrompt{false};
  bool shouldSpin{false};
  bool shouldStop{false};
  bool shouldClose{false};
  bool shouldThrow{false};

  void onCreate() override {
    phasor.frequency(220);
    gui.init();
    gui << active << value;
    audioIO().print();
  }

  void onAnimate(double dt) override {
    navControl().active(!gui.usingInput());
    //
  }

  void onDraw(Graphics& g) override {
    g.clear(0.3);
    gui.draw(g);
  }

  void onSound(AudioIOData& io) override {
    int i = active.get() + 1;
    if (i >= 1000) {
      i = 0;
    }
    active.set(i);

    while (io()) {
      float f = tri(phasor()) * 0.1;
      value.set(f);
      io.out(0) = f;
      io.out(1) = f;

      // ways to fail
      //
      //

      if (shouldSpin) {
        // do nothing useful, really fast
        while (true)
          ;
      }

      if (shouldPrompt) {
        getchar();  // wait for user to type something
      }

      if (shouldStop) {
        audioIO().stop();
      }

      if (shouldClose) {
        audioIO().close();
      }

      if (shouldThrow) {
        throw std::out_of_range("got here");
      }
    }
  }

  bool onKeyDown(const Keyboard& k) override {
    switch (k.key()) {
      case 's':
        shouldSpin = true;
        break;

      case 'p':
        shouldPrompt = true;
        break;

      case 't':
        shouldStop = true;
        break;

      case 'c':
        shouldClose = true;
        break;

      case 'w':
        shouldThrow = true;
        break;

      default:
        break;
    }

    return true;
  }
};

int main() {
  MyApp app;
  app.configureAudio(SAMPLE_RATE, BLOCK_SIZE, OUTPUT_CHANNELS);
  app.start();
}
