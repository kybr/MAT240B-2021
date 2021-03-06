// Karl Yerkes / 2021-02-18 / MAT240B
//
// Ways to fail in the audio thread
//
// This example does several of the things we generally don't want to do in the
// context of an audio thread.
//
// See
// http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing
//
// - Don’t allocate or deallocate memory
// - Don’t lock a mutex
// - Don’t read or write to the filesystem or otherwise perform i/o. (In case
// there’s any doubt, this includes things like calling printf or NSLog, or GUI
// APIs.)
// - Don’t call OS functions that may block waiting for something
// - Don’t execute any code that has unpredictable or poor worst-case timing
// behavior
// - Don’t call any code that does or may do any of the above
// - Don’t call any code that you don’t trust to follow these rules
// - On Apple operating systems follow Apple’s guidelines
//
// - Do use algorithms with good worst-case time complexity (ideally O(1)
// wost-case)
// - Do amortize computation across many audio samples to smooth out CPU usage
// rather than using “bursty” algorithms that occasionally have long processing
// times
// - Do pre-allocate or pre-compute data in a non-time-critical thread
// - Do employ non-shared, audio-callback-only data structures so you don’t need
// to think about sharing, concurrency and locks
//

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
  bool shouldMemory{false};

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
    try {
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
          shouldThrow = false;
          throw std::out_of_range("EXCEPTION!!!");
          // at(0) on an empty std:vector<float> we get an std::out_of_range
        }

        if (shouldMemory) {
          shouldMemory = false;
          float* memory = new float[1e10];
          memory[0] = 1;
          delete[] memory;
        }
      }

    } catch (...) {
      printf("got here\n");
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

      case 'm':
        shouldMemory = true;
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
