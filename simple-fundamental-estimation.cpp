#include "Gamma/DFT.h"  // gam::STFT
#include "Gamma/Filter.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"  // gui.draw(g)
using namespace al;

#include <limits>

#include "functions.h"

gam::STFT stft(4096, 4096 / 4, 16384, gam::HAMMING);
gam::SamplePlayer<float, gam::ipl::Cubic, gam::phsInc::Loop> player;

struct MyApp : App {
  float hz;
  gam::Sine<> osc;
  gam::OnePole<> frequencyFilter, rateFilter;
  Parameter rate{"/rate", "", 1.0, "", 0.0, 2.0};
  ControlGUI gui;

  Mesh spectrum{Mesh::LINE_STRIP};
  Mesh line{Mesh::LINES};

  float minimum{std::numeric_limits<float>::max()};
  float maximum{-std::numeric_limits<float>::max()};

  void onCreate() override {
    gui << rate;
    gui.init();
    navControl().useMouse(false);

    frequencyFilter.freq(25);
    rateFilter.freq(25);

    gam::sampleRate(audioIO().framesPerSecond());
    player.load("../BBQ.wav");

    for (int i = 0; i < stft.numBins(); i++) {
      spectrum.vertex(2.0 * i / stft.numBins() - 1);
    }

    line.vertex(0, 1);
    line.vertex(0, -1);
  }

  void onSound(AudioIOData& io) override {
    while (io()) {
      player.rate(rateFilter(rate.get()));
      float f = player();

      if (stft(f)) {
        //
        // search for the "hottest" bin
        int hottest = -1;
        float _maximum = -std::numeric_limits<float>::max();

        int N = stft.numBins();
        for (int i = 0; i < N; i++) {
          //
          // calculate statistics on the magnitude of the bin
          float m = log(stft.bin(i).mag());
          if (m > maximum) maximum = m;
          if (m < minimum) minimum = m;
          spectrum.vertices()[i].y = diy::map(m, minimum, maximum, -1, 1);

          // search for the "hottest" bin
          if (m > _maximum) {
            _maximum = m;
            hottest = i;
          }
        }

        // if a hottest bin was found on this frame
        if (hottest > -1) {
          hz = audioIO().framesPerSecond() / 2.0f * hottest / N;
        }
      }

      osc.freq(frequencyFilter(hz));
      io.out(0) = f;
      io.out(1) = osc() * 0.5;
    }
  }

  bool onKeyDown(const Keyboard& k) override {
    minimum = std::numeric_limits<float>::max();
    maximum = -std::numeric_limits<float>::max();
  }

  void onDraw(Graphics& g) override {
    g.clear(0.3);
    g.camera(Viewpoint::IDENTITY);  // Ortho [-1:1] x [-1:1]
    g.color(0, 1, 1);
    g.draw(spectrum);
    g.color(1, 0, 0);
    g.translate(diy::map(hz, 0, audioIO().framesPerSecond() / 2, -1, 1), 0, 0);
    g.draw(line);
    gui.draw(g);
  }
};

int main() {
  MyApp app;
  app.start();
}
