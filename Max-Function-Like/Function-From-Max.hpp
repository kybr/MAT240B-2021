#pragma once

// inlcude this before MyApp
// #inlcude "Function-From-Max.hpp"

struct MyApp;
class FunctionFromMax {
  MyApp* app;
  Mesh circle;
  Mesh line;
  std::vector<Vec3f> point;
  std::vector<bool> hover;

  //
  // Helper Methods
  //

  Vec3d unproject(Vec3d screenPos);
  Rayd getPickRay(int screenX, int screenY);

 public:
  FunctionFromMax(MyApp* that);

  // set of pairs or (value,time) where time and value are normalized on (0, 1)
  std::vector<al::Vec2f> points();

  void onCreate();
  void onDraw(Graphics& g);
  bool onMouseDrag(const Mouse& m);
  bool onMouseDown(const Mouse& m);
  bool onMouseUp(const Mouse& m);
  bool onMouseMove(const Mouse& m);
};

