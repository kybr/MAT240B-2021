// Karl Yerkes / 2021-03-01 / MAT240B
//
// Title: Scatter Sequence
//
// Interactive selection of elements of a musical sequence
//
// Hovering plays an item immediately while clicking adds the element to the
// sequence.
//
// Yet to be done:
// - choose a better container for sequence
//   + std::vector has high complexity for the find and erase operations
// - add comments explaining use of std::mutex
// - encapsulate into Sequence class?
// - use mtof rather than linear frequency mapping
// - put the rate control on a parameter
// - animate the playback of the sequence
//
#include "Gamma/Oscillator.h"
#include "al/app/al_App.hpp"
#include "al/math/al_Functions.hpp"  // al::clip
#include "al/math/al_Random.hpp"
#include "al/math/al_Ray.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace al;

#include <mutex>  // try_lock

struct Grain : SynthVoice {
  gam::SineD<> sine;

  void onProcess(AudioIOData& io) override {
    while (io()) {
      float f = sine();
      io.out(0) += f;
      io.out(1) += f;

      if (sine.phase() > 1) {
        free();
        break;
      }
    }
  }
};

struct MyApp : App {
  Mesh circle;

  std::vector<Vec3f> point;
  std::vector<bool> hover;
  float rate{1};
  PolySynth polySynth;

  // we need a mutex to protect these two variables
  std::mutex mutex;
  std::vector<Vec3f> sequence;
  int index{0};

  gam::Accum<> timer;

  void onCreate() override {
    timer.freq(4);

    polySynth.allocatePolyphony<Grain>(10);
    nav().pos(0, 0, 10);

    // make a circle mesh
    circle.primitive(Mesh::TRIANGLES);
    const int N = 100;
    for (int i = 1; i < N + 1; i++) {
      float a0 = i * M_PI * 2 / N;
      float a1 = (i - 1) * M_PI * 2 / N;
      float r = 0.1;
      circle.vertex(0, 0);
      circle.vertex(r * sin(a0), r * cos(a0));
      circle.vertex(r * sin(a1), r * cos(a1));
    }

    rnd::Random<> rng;
    rng.seed(100);

    for (int i = 0; i < 24; i++) {
      point.emplace_back();
      point.back().x = rng.uniform(1.0, -1.0);
      point.back().y = rng.uniform(1.0, -1.0);
      point.back().z = 0.0;
    }

    hover.resize(point.size(), false);

    //
    navControl().useMouse(false);

    printf("%lu\n", point.size());
  }

  virtual void onDraw(Graphics& g) override {
    g.clear(0.27);

    // for (int i = 0; i < point[i].size(); i++) {
    // (the line above compiles, but it's so wrong
    for (int i = 0; i < point.size(); i++) {
      g.pushMatrix();
      g.color(hover[i] ? 0.5 : 1.0);
      g.translate(point[i]);
      g.draw(circle);
      g.popMatrix();
    }
  }

  bool onMouseDrag(const Mouse& m) override {  //
    return true;
  }

  bool onMouseDown(const Mouse& m) override {  //
    Rayd r = getPickRay(m.x(), m.y());

    for (int i = 0; i < point.size(); i++) {
      float t = r.intersectSphere(point[i], 0.1);

      if (t > 0.0f) {
        mutex.lock();  // blocks

        bool found = false;
        for (auto iterator = sequence.begin(); iterator != sequence.end();) {
          if (*iterator == point[i]) {
            found = true;

            // actually, this would erase ALL the copies of point[i] in
            // sequence.
            sequence.erase(iterator);
          } else {
            ++iterator;  // only advance the iterator when we don't erase
          }
        }

        if (!found) {
          sequence.push_back(point[i]);
          //
        }

        mutex.unlock();
      }
    }
    return true;
  }

  bool onMouseMove(const Mouse& m) override {
    Rayd r = getPickRay(m.x(), m.y());

    for (int i = 0; i < point.size(); i++) {
      float t = r.intersectSphere(point[i], 0.1);

      // only trigger once; no re-trigger when hovering
      if (!hover[i] && t > 0.0f) {
        // trigger grain
        Vec3f v = point[i];
        auto* voice = polySynth.getVoice<Grain>();
        float frequency = mapRange(v.x, -1.0f, 1.0f, 100.0f, 800.0f);
        float duration = mapRange(v.y, -1.0f, 1.0f, 0.1f, 1.1f);
        voice->sine.set(frequency, 0.2, duration);
        polySynth.triggerOn(voice);
      }
      hover[i] = t > 0.f;
    }
    return true;
  }

  void onSound(AudioIOData& io) override {
    gam::sampleRate(audioIO().framesPerSecond());

    //
    polySynth.render(io);

    //
    io.frame(0);

    while (io()) {
      if (timer()) {
        // try_lock does not block, but may fail
        if (mutex.try_lock()) {
          if (sequence.size() > 0) {
            Vec3f v = sequence[index];

            auto* voice = polySynth.getVoice<Grain>();
            float frequency = mapRange(v.x, -1.0f, 1.0f, 100.0f, 800.0f);
            float duration = mapRange(v.y, -1.0f, 1.0f, 0.1f, 1.1f);
            voice->sine.set(frequency, 0.9, duration);
            polySynth.triggerOn(voice);

            index++;
            if (index >= sequence.size()) {
              index -= sequence.size();
              if (index < 0) {
                index = 0;
              }
            }
          }

          mutex.unlock();
        }
      }
    }
  }

  //
  // Helper Methods
  //

  Vec3d unproject(Vec3d screenPos) {
    auto& g = graphics();
    auto mvp = g.projMatrix() * g.viewMatrix() * g.modelMatrix();
    Matrix4d invprojview = Matrix4d::inverse(mvp);
    Vec4d worldPos4 = invprojview.transform(screenPos);
    return worldPos4.sub<3>(0) / worldPos4.w;
  }

  Rayd getPickRay(int screenX, int screenY) {
    Rayd r;
    Vec3d screenPos;
    screenPos.x = (screenX * 1. / width()) * 2. - 1.;
    screenPos.y = ((height() - screenY) * 1. / height()) * 2. - 1.;
    screenPos.z = -1.;
    Vec3d worldPos = unproject(screenPos);
    r.origin().set(worldPos);

    screenPos.z = 1.;
    worldPos = unproject(screenPos);
    r.direction().set(worldPos);
    r.direction() -= r.origin();
    r.direction().normalize();
    return r;
  }
};

int main() {
  MyApp app;
  app.configureAudio(44100, 512, 2, 0);
  app.start();
}
