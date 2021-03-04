#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
using namespace al;

#include "synths.h"
using namespace diy;

#include <functional>
#include <vector>
using namespace std;

struct Operator : Phasor {
  float r = 1;  // frequency ratio
  float s = 1;  // scale factor
  ADSR eg;      // envelope generator

  // without modulation
  float operator()() {
    // sine() is now a function that does a table lookup
    // look in synths.h for more information.
    return sine(Phasor::operator()()) * eg() * s;
  }

  // with modulation
  float operator()(float modulation) {
    modulate(modulation);  // adds frequency to a Phasor
    return operator()();
  }

  void print() {
    printf("  frequency:%f\n", frequency());
    printf("  ratio:%f\n", r);
    printf("  scale:%f\n", s);
    eg.print();
  }
};

struct DX7 {
  // This is a C++ pattern that allows us to refer to the 6 operators
  // both as an array and as letters (a, b, c, d, etc.)
  /*
   * this fails in gcc in Linux :(
  union {
    struct {
      Operator a, b, c, d, e, f;
    };
    Operator op[6];
  };
  */
  Operator op[6];
  Operator &a = op[0], &b = op[1], &c = op[2], &d = op[3], &e = op[4],
           &f = op[5];

  // vector<function<float(void)>> fun;
  // inline float foo() { return (a(b()) + c(d(e(_ = f(_))))) / 2; }

  DX7() {
    for (auto& o : op) {
      o.r = 1;
      o.s = 1;
      o.eg.set(0, 0, 1, 0);
      o.eg.state = 0;
    }
  }

  void randomize(bool harmonic, float scale) {
    for (auto& o : op) {
      if (harmonic)
        o.r = rnd::uniform(8, 1);
      else
        o.r = rnd::uniform(2.0, 1.0);
      o.s = mtof(rnd::uniform(scale));
      o.eg.set(rnd::uniform(0.8, 0.05), rnd::uniform(0.3, 0.1),
               rnd::uniform(0.9, 0.4), rnd::uniform(1.1, 0.2));
    }
  }

  void frequency(float hertz) {
    for (auto& o : op) o.frequency(hertz * o.r);
  }

  void on() {
    for (auto& o : op) {
      o.eg.on();
      o.value = 0;
    }
  }

  void off() {
    for (auto& o : op) o.eg.off();
  }

  int algorithm = 32;
  float _ = 0;  // feedback parameter
  float x;

  float operator()() {
    switch (algorithm) {
      default:
      case 1:
        _ = f(_);
        return (a(b()) + c(d(e(_)))) / (a.s + c.s);
      case 2:
        return (a(_ = b(_)) + c(d(e(f())))) / (a.s + c.s);
      case 3:
        return (a(b(c())) + d(e(_ = f(_)))) / (a.s + d.s);
      case 4:
        return (a(b(c())) + (_ = c(d(f(_))))) / (a.s + d.s);
      case 5:
        return (a(b()) + c(d()) + e(_ = f(_))) / (a.s + c.s + e.s);
      case 6:
        return (a(b()) + c(d()) + (_ = e(f(_)))) / (a.s + c.s + e.s);
      case 7:
        return (a(b()) + c(d() + e(_ = f(_)))) / (a.s + c.s);
      case 8:
        return (a(b()) + c((_ = d(_)) + e(f()))) / (a.s + c.s);
      case 9:
        return (a(_ = b(_)) + c(d() + e(f()))) / (a.s + c.s);
      case 10:
        return (a(b(_ = c(_))) + d(e() + f())) / (a.s + d.s);
      case 11:
        return (a(b(c())) + d(e() + (_ = f(_)))) / (a.s + d.s);
      case 12:
        return (a(_ = b(_)) + c(d() + e() + f())) / (a.s + c.s);
      case 13:
        return (a(b()) + c(d() + e() + (_ = f(_)))) / (a.s + c.s);
      case 14:
        return (a(b()) + c(d(e() + (_ = f(_))))) / (a.s + c.s);
      case 15:
        return (a(_ = b(_)) + c(d(e() + f()))) / (a.s + c.s);
      case 16:
        return (a(b() + c(d()) + e(_ = f(_)))) / a.s;
      case 17:
        return (a((_ = b(_)) + c(d()) + e(f()))) / a.s;
      case 18:
        return (a(b() + (_ = c(_)) + d(e(f())))) / a.s;
      case 19:
        _ = f(_);
        return (a(b(c())) + d(_) + e(_)) / (a.s + d.s + e.s);
      case 20:
        _ = c(_);
        return (a(_) + b(_) + d(e() + f())) / (a.s + b.s + d.s);
      case 21:
        _ = c(_);
        x = f();
        return (a(_) + b(_) + d(x) + e(x)) / (a.s + b.s + d.s + e.s);
      case 22:
        _ = f(_);
        return (a(b()) + c(_) + d(_) + e(_));
      case 23:
        _ = f(_);
        return (a() + b(c()) + d(_) + e(_)) / (a.s + b.s + d.s + e.s);
      case 24:
        _ = f(_);
        return (a() + b() + c(_) + d(_) + e(_)) / (a.s + b.s + c.s + d.s + e.s);
      case 25:
        _ = f(_);
        return (a() + b() + c() + d(_) + e(_)) / (a.s + b.s + c.s + d.s + e.s);
      case 26:
        return (a() + b(c()) + d(e() + (_ = f(_)))) / (a.s + b.s + d.s);
      case 27:
        return (a() + b(_ = c(_)) + d(e() + f())) / (a.s + b.s + d.s);
      case 28:
        return (a(b()) + c(d(_ = e(_))) + f()) / (a.s + c.s + f.s);
      case 29:
        return (a() + b() + c(d()) + e(_ = f(_))) / (a.s + b.s + c.s + d.s);
      case 30:
        return (a() + b() + c(d(_ = e(_))) + f()) / (a.s + b.s + c.s + f.s);
      case 31:
        return (a() + b() + c() + d() + e(_ = f(_))) /
               (a.s + b.s + c.s + d.s + e.s);
      case 32:
        return (a() + b() + c() + d() + e() + (_ = f(_))) /
               (a.s + b.s + c.s + d.s + e.s + f.s);
    }
  }

  void print() {
    printf("\033[2J\033[;H");
    printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    printf("XX  DX7 Settings  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    // printf("algorithm:%d\n", algorithm);
    for (auto& o : op) {
      printf("  ----------\n");
      o.print();
    }
    fflush(stdout);
  }
};

struct MyApp : App {
  ControlGUI gui;
  Parameter frequency{"/frequency", "", 40, "", 0, 127};
  ParameterInt algorithm{"/algorithm", "", 32, "", 1, 32};
  Parameter scale{"/scale", "", 1, "", 0, 50};
  ParameterInt note{"/note", "", 32, "", 18, 48};

  DX7 dx7;

  void onCreate() override {
    rnd::global().seed(21);
    gui.init();
    gui.add(algorithm);
    gui.add(frequency);
    gui.add(scale);
    gui.add(note);

    dx7.randomize(true, 10);
    dx7.print();
  }

  void onAnimate(double dt) override {
    //
    navControl().active(!gui.usingInput());
  }

  void onDraw(Graphics& g) override {
    g.clear(0.25);
    gui.draw(g);
  }

  int last = 0;
  void onSound(AudioIOData& io) override {
    dx7.algorithm = algorithm;
    if (last != note) {
      last = note;
      dx7.frequency(mtof(note));
    }
    while (io()) {
      float f = dx7() / 2;
      io.out(0) = f;
      io.out(1) = f;
    }
  }

  bool onKeyDown(Keyboard const& k) override {
    if (k.key() == ' ') dx7.on();
    if (k.key() == 'h') dx7.randomize(true, scale);
    if (k.key() == 'i') dx7.randomize(false, scale);
    return true;
  }

  bool onKeyUp(Keyboard const& k) override {
    if (k.key() == ' ') dx7.off();
    return true;
  }
};

int main() {
  MyApp app;
  app.configureAudio(48000, 512, 2, 0);
  app.start();
}
