#pragma once

// inlcude this **after** MyApp
// #inlcude "Function-From-Max.cpp"

FunctionFromMax::FunctionFromMax(MyApp* that) : app(that) {}

Vec3d FunctionFromMax::unproject(Vec3d screenPos) {
  auto& g = app->graphics();
  auto mvp = g.projMatrix() * g.viewMatrix() * g.modelMatrix();
  Matrix4d invprojview = Matrix4d::inverse(mvp);
  Vec4d worldPos4 = invprojview.transform(screenPos);
  return worldPos4.sub<3>(0) / worldPos4.w;
}

Rayd FunctionFromMax::getPickRay(int screenX, int screenY) {
  Rayd r;
  Vec3d screenPos;
  screenPos.x = (screenX * 1. / app->width()) * 2. - 1.;
  screenPos.y = ((app->height() - screenY) * 1. / app->height()) * 2. - 1.;
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

float FunctionFromMax::evaluate(float pixel) {
  //
  //
  return 0;
}

void FunctionFromMax::onCreate() {
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
    point.back().x = rng.uniform(0, app->width());
    point.back().y = rng.uniform(0, app->height());
    point.back().z = 0.0;
  }

  hover.resize(point.size(), false);
}

void FunctionFromMax::onDraw(Graphics& g) {
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

bool FunctionFromMax::onMouseDrag(const Mouse& m) {  //
  if (app->gui.usingInput()) return false;

  Rayd r = getPickRay(m.x(), m.y());

  for (int i = 0; i < point.size(); i++) {
    float t = r.intersectSphere(point[i], 10);

    if (t > 0.0f) {
      point[i].x = m.x();
      point[i].y = app->height() - m.y();
      break;  // only handle the first to intersect
    }
  }

  return false;
}

bool FunctionFromMax::onMouseDown(const Mouse& m) {  //
  if (app->gui.usingInput()) return false;

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
    point.back().y = app->height() - m.y();
  }

  return false;
}

bool FunctionFromMax::onMouseUp(const Mouse& m) {  //
  if (app->gui.usingInput()) return false;

  sort(point.begin(), point.end(),
       [](const Vec3f& a, const Vec3f& b) { return a.x < b.x; });
  line.vertices().clear();
  for (int i = 0; i < point.size(); i++) {
    line.vertex(point[i]);
  }
  return false;
}

bool FunctionFromMax::onMouseMove(const Mouse& m) {
  if (app->gui.usingInput()) return false;

  Rayd r = getPickRay(m.x(), m.y());

  for (int i = 0; i < point.size(); i++) {
    float t = r.intersectSphere(point[i], 10);
    hover[i] = t > 0.f;
  }
  return false;
}
