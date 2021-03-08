// Karl Yerkes / 2021-03-02 / MAT240B
//
// Title: Max Function Like
//
// Interactive selection of elements of a piecewise linear function
//
// Yet to be done:
// - MultiLine class for audio envelope
// - zoom/pan
//
#include "Gamma/Oscillator.h"
#include "al/app/al_App.hpp"
#include "al/math/al_Functions.hpp"  // al::clip
#include "al/math/al_Random.hpp"
#include "al/math/al_Ray.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace al;

#include "Function-From-Max.hpp"

struct MyApp : App {
  ControlGUI gui;
  Parameter time{"/time", "", 0, "", 0, 100000};
  Parameter value{"/value", "", 0, "", 0, 1};
  FunctionFromMax functionFromMax{this};

  void onCreate() override {
    functionFromMax.onCreate();

    gui.init();
    gui << time;
    gui << value;

    nav().pos(0, 0, 10);
    //
    navControl().useMouse(false);
  }

  void onAnimate(double dt) override {
    navControl().active(!gui.usingInput());
    //
  }

  virtual void onDraw(Graphics& g) override {
    g.clear(0.27);

    functionFromMax.onDraw(g);
    gui.draw(g);
  }

  void onSound(AudioIOData& io) override {
    gam::sampleRate(audioIO().framesPerSecond());

    while (io()) {
      // float s = sine();
      // io.out(0) = s * gain;
      // io.out(1) = s * gain;
    }
  }

  //
  // delegations...
  //

  bool onMouseDrag(const Mouse& m) override {
    functionFromMax.onMouseDrag(m);
    return false;
  }

  bool onMouseDown(const Mouse& m) override {
    functionFromMax.onMouseDown(m);
    return false;
  }

  bool onMouseUp(const Mouse& m) override {
    functionFromMax.onMouseUp(m);
    return false;
  }

  bool onMouseMove(const Mouse& m) override {
    functionFromMax.onMouseMove(m);
    return false;
  }
};

int main() {
  MyApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}

#include "Function-From-Max.cpp"
