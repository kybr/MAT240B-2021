#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace al;
struct MyApp : App {
  Parameter mode{"mode", "", 0.0, "", 0.0f, 1.0f};
  ControlGUI gui;

  MyApp(int argc, char *argv[]) {
    // C++ "constructor" called when MyApp is declared
    //
  }

  void onInit() override {
    // called a single time just after the app is started
    //
  }

  void onCreate() override {
    // called a single time some time
    //

    nav().pos(Vec3d(0, 0, 8));  // Set the camera to view the scene

    gui << mode;
    gui.init();

    // Disable nav control; So default keyboard and mouse control is disabled
    navControl().active(false);
  }

  void onAnimate(double dt) override {}

  void onDraw(Graphics &g) override {
    g.clear();
    //
    //

    // Draw th GUI
    gui.draw(g);
  }

  void onSound(AudioIOData &io) override {
    while (io()) {
      float f = io.in(0) * 0.0;
      io.out(0) = f;
      io.out(1) = f;
    }
  }

  bool onKeyDown(const Keyboard &k) override {
    int midiNote = asciiToMIDI(k.key());
    return true;
  }

  bool onKeyUp(const Keyboard &k) override {
    int midiNote = asciiToMIDI(k.key());
    return true;
  }
};

int main(int argc, char *argv[]) {
  // MyApp constructor called here, given arguments from the command line
  MyApp app(argc, argv);

  // Start the AlloLib framework's "app" construct. This blocks until the app is
  // quit (or it crashes).
  app.start();

  return 0;
}
