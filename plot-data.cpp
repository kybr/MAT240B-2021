#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace al;
struct MyApp : App {
  Parameter mode{"mode", "", 0.0, "", 0.0f, 1.0f};
  Parameter scaleX{"scaleX", "", 1.0, "", 0.4f, 2.1f};
  Parameter scaleY{"scaleY", "", 1.0, "", 0.4f, 2.1f};
  ControlGUI gui;

  Mesh data;
  double minimum = 1e30;
  double maximum = -1e30;

  MyApp(int argc, char *argv[]) {
    std::vector<double> values;
    double value;
    while (std::cin >> value) {
      if (value > maximum) {
        maximum = value;
      }
      if (value < minimum) {
        minimum = value;
      }
      values.push_back(value);
    }

    for (int i = 0; i < values.size(); i++) {
      data.vertex(i, values[i]);  // x, y
    }

    data.primitive(Mesh::LINE_STRIP);

    printf("%lf, %lf\n", minimum, maximum);
  }

  void onCreate() override {
    gui << mode;
    gui << scaleX;
    gui << scaleY;
    gui.init();

    // Disable nav control; So default keyboard and mouse control is disabled
    navControl().active(false);
  }

  void onDraw(Graphics &g) override {
    g.camera(Viewpoint::ORTHO_FOR_2D);  // Ortho [0:width] x [0:height]
    g.clear(mode);

    g.color(1 - mode);
    g.translate(0, height() / 2, 0);
    g.scale(scaleX * width() / data.vertices().size(),
            scaleY * height() / (maximum - minimum));
    g.draw(data);

    // Draw th GUI
    gui.draw(g);
  }

  bool onMouseScroll(Mouse const &mouse) override {
    printf("%lf, %lf (scroll)\n", mouse.scrollX(), mouse.scrollY());
    return true;
  }

  bool onMouseMove(Mouse const &mouse) override {
    printf("%d, %d (move)\n", mouse.x(), mouse.y());
    return true;
  }

  bool onMouseDrag(Mouse const &mouse) override {
    printf("%d, %d (drag)\n", mouse.x(), mouse.y());
    return true;
  }
};

int main(int argc, char *argv[]) {
  MyApp app(argc, argv);
  app.start();
  return 0;
}
