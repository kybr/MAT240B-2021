// Karl Yerkes / 2021-03-01 / MAT240B
// Karl Yerkes / 2021-03-02 / MAT240B
//
// Title: Max Function Like
//
// Interactive selection of elements of a piecewise linear function
//
// Yet to be done:
// - MultiLine class for audio envelope
// - encapsulate into an al::DrawFunctionUI thing
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

struct MyApp : App {
  Mesh circle;
  Mesh line;

  std::vector<Vec3f> point;
  std::vector<bool> hover;

  void onCreate() override {
    nav().pos(0, 0, 10);
    line.primitive(Mesh::LINE_STRIP);

    // make a circle mesh
    circle.primitive(Mesh::TRIANGLES);
    const int N = 100;
    for (int i = 1; i < N + 1; i++) {
      float a0 = i * M_PI * 2 / N;
      float a1 = (i - 1) * M_PI * 2 / N;
      float r = 10;
      circle.vertex(0, 0);
      circle.vertex(r * sin(a0), r * cos(a0));
      circle.vertex(r * sin(a1), r * cos(a1));
    }

    rnd::Random<> rng;
    rng.seed(100);

    // maybe make some data
    for (int i = 0; i < 0; i++) {
      point.emplace_back();
      point.back().x = rng.uniform(0, width());
      point.back().y = rng.uniform(0, height());
      point.back().z = 0.0;
    }

    hover.resize(point.size(), false);

    //
    navControl().useMouse(false);
  }

  virtual void onDraw(Graphics& g) override {
    g.clear(0.27);
    g.camera(Viewpoint::ORTHO_FOR_2D);  // Ortho [0:width] x [0:height]

    // for (int i = 0; i < point[i].size(); i++) {
    // (the line above compiles, but it's so wrong
    for (int i = 0; i < point.size(); i++) {
      g.pushMatrix();
      g.color(hover[i] ? 0.5 : 1.0);
      g.translate(point[i]);
      g.draw(circle);
      g.popMatrix();
    }

    g.color(1.0);
    g.draw(line);
  }

  bool onMouseDrag(const Mouse& m) override {  //
    Rayd r = getPickRay(m.x(), m.y());

    for (int i = 0; i < point.size(); i++) {
      float t = r.intersectSphere(point[i], 10);

      if (t > 0.0f) {
        point[i].x = m.x();
        point[i].y = height() - m.y();
        break;  // only handle the first to intersect
      }
    }
    return true;
  }

  bool onMouseDown(const Mouse& m) override {  //
    Rayd r = getPickRay(m.x(), m.y());

    bool found = false;
    for (int i = 0; i < point.size(); i++) {
      float t = r.intersectSphere(point[i], 10);

      if (t > 0.0f) {
        found = true;
        break;  // only handle the first to intersect
      }
    }

    if (!found) {
      point.emplace_back();
      hover.emplace_back();
      point.back().x = m.x();
      point.back().y = height() - m.y();
    }
    return true;
  }

  bool onMouseUp(const Mouse& m) override {  //
    sort(point.begin(), point.end(),
         [](const Vec3f& a, const Vec3f& b) { return a.x < b.x; });
    line.vertices().clear();
    for (int i = 0; i < point.size(); i++) {
      line.vertex(point[i]);
    }
    return true;
  }

  bool onMouseMove(const Mouse& m) override {
    Rayd r = getPickRay(m.x(), m.y());

    for (int i = 0; i < point.size(); i++) {
      float t = r.intersectSphere(point[i], 10);
      hover[i] = t > 0.f;
    }
    return true;
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
