#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace al;

#include "synths.h"
using namespace diy;

#include <vector>
using namespace std;

double vowel[] = {
    1.000000000000000, 0.696969696969697, 0.490909090909092, 2.400000000000000,
    2.400000000000000, 2.400000000000000, 3.200000000000000, 4.139393939393939,
    4.200000000000000, 4.200000000000000, 3.457575757575757, 2.850000000000000,
    2.319696969696970, 1.796969696969697, 1.645454545454546, 1.600000000000000,
    1.254545454545454, 0.800000000000000, 0.800000000000000, 1.672727272727273,
    2.000000000000000, 3.200000000000000, 3.200000000000000,
};

struct Tube {
  // model parameters
  //
  double spaceStep{0};
  double timeStep{0};
  double waveSpeed{0};
  double tractLength{0};
  double tractSurfaceAreaLeftEnd{0};

  // TODO meaningful names? may not be possible
  double gamma{0};
  double lambda{0};
  double s0{0};
  double r1{0};
  double r2{0};
  double g1{0};

  int N;  // number of sections

  // state
  //
  double* p0{nullptr};
  double* p1{nullptr};
  double* p2{nullptr};
  double* s{nullptr};

  void set(int length) {
    N = length;

    if (p0) delete[] p0;
    if (p1) delete[] p1;
    if (p2) delete[] p2;
    if (s) delete[] s;

    p0 = new double[N];
    p1 = new double[N];
    p2 = new double[N];
    s = new double[N];

    timeStep = 1.0 / 44100;                 // sceonds
    spaceStep = 1.0 / (N - 1);              // no units
    waveSpeed = 340.0;                      // meter/second
    tractLength = 0.17;                     // meters
    tractSurfaceAreaLeftEnd = 0.00025;      // square meters
    gamma = waveSpeed / tractLength;        // Hertz, 1 / seconds
    lambda = gamma * timeStep / spaceStep;  // no units

    reset();

    recalculate();
  }

  void reset() {
    for (int i = 0; i < N; ++i) {
      p0[i] = p1[i] = p2[i] = 0.0;
      s[i] = 1;
      // s[i] = vowel[i];
    }
  }

  void recalculate() {
    // TODO :: if possible, make each variable name meaningful
    double alf =
        2.0881 * tractLength * sqrt(1.0 / (tractSurfaceAreaLeftEnd * s[N - 1]));
    double bet = 0.7407 / gamma;
    double Sr = 1.5 * s[N - 1] - 0.5 * s[N - 2];
    double q1 =
        alf * gamma * gamma * timeStep * timeStep * Sr / (s[N - 1] * spaceStep);
    double q2 = bet * gamma * gamma * timeStep * Sr / (s[N - 1] * spaceStep);
    s0 = 2.0 * (1.0 - lambda * lambda);
    r1 = 2.0 * lambda * lambda / (1.0 + q1 + q2);
    r2 = -(1.0 + q1 - q2) / (1.0 + q1 + q2);
    g1 = -(timeStep * timeStep * gamma * gamma / spaceStep / s[0]) *
         (3.0 * s[0] - s[1]);
  }

  double operator()(double excitation) {
    // calculate the body of the tube
    //
    for (int k = 1; k < N - 1; ++k) {
      double mean = 0.25 * (s[k + 1] + s[k] + s[k] + s[k - 1]);
      p0[k] = 2.0 * (1.0 - lambda * lambda) * p1[k] +
              (0.5 * lambda * lambda / mean) * ((s[k] + s[k - 1]) * p1[k - 1] +
                                                (s[k] + s[k + 1]) * p1[k + 1]) -
              p2[k];
    }

    // calculate the right (radiation) end of the tube
    //
    p0[N - 1] = r1 * p1[N - 2] + r2 * p2[N - 1];

    // calculate the left (excitation) end of the tube
    //
    p0[0] = 2.0 * (1.0 - lambda * lambda) * p1[0] +
            2.0 * lambda * lambda * p1[1] - p2[0] + g1 * excitation;

    // sample the right end of the tube
    //
    double output = (p0[N - 1] - p1[N - 1]) / timeStep;

    double* temp = p2;
    p2 = p1;
    p1 = p0;
    p0 = temp;

    return output;
  }
};

Parameter vocal[]{
    {"/vocal1", "", 1, "", 0, 2.0},  {"/vocal2", "", 1, "", 0, 2.0},
    {"/vocal3", "", 1, "", 0, 2.0},  {"/vocal4", "", 1, "", 0, 2.0},
    {"/vocal5", "", 1, "", 0, 2.0},  {"/vocal6", "", 1, "", 0, 2.0},
    {"/vocal7", "", 1, "", 0, 2.0},  {"/vocal8", "", 1, "", 0, 2.0},
    {"/vocal9", "", 1, "", 0, 2.0},  {"/vocal10", "", 1, "", 0, 2.0},
    {"/vocal11", "", 1, "", 0, 2.0}, {"/vocal12", "", 1, "", 0, 2.0},
    {"/vocal13", "", 1, "", 0, 2.0}, {"/vocal14", "", 1, "", 0, 2.0},
    {"/vocal15", "", 1, "", 0, 2.0}, {"/vocal16", "", 1, "", 0, 2.0},
    {"/vocal17", "", 1, "", 0, 2.0}, {"/vocal18", "", 1, "", 0, 2.0},
    {"/vocal19", "", 1, "", 0, 2.0}, {"/vocal20", "", 1, "", 0, 2.0},
    {"/vocal21", "", 1, "", 0, 2.0}, {"/vocal22", "", 1, "", 0, 2.0},
    {"/vocal23", "", 1, "", 0, 2.0}};

struct MyApp : App {
  Tube tube;
  Sine oscillator;
  ControlGUI gui;

  Parameter frequency{"/frequency", "", 48, "", 0, 127};

  void onCreate() override {
    gui.init();
    oscillator.frequency(100);
    tube.set(23);

    gui << frequency;
    for (auto& v : vocal) gui << v;
  }

  void onAnimate(double dt) override {
    navControl().active(!gui.usingInput());
    //
  }

  void onDraw(Graphics& g) override {
    g.clear(0.5);
    gui.draw(g);
  }

  void onSound(AudioIOData& io) override {
    int n = 0;
    for (auto& v : vocal) tube.s[n++] = v;
    oscillator.frequency(mtof(frequency));
    while (io()) {
      float s = oscillator() / 20;
      float f = tube(s > 0 ? s : 0) * 0.1;
      io.out(0) = f;
      io.out(1) = f;
    }
  }
};

int main() {
  MyApp app;
  app.configureAudio(SAMPLE_RATE, BLOCK_SIZE, OUTPUT_CHANNELS, INPUT_CHANNELS);
  app.start();
}
