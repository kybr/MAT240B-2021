#pragma once

#include <cmath>  // sin, cos, log
#include <vector>

const double SAMPLE_RATE = 48000;
const double pi =
    3.141592653589793238462643383279502884197169399375105820974944592307816406286;
const double e =
    2.718281828459045235360287471352662497757247093699959574966967627724076630353;

double mtof(double m) { return 440.0 * pow(2.0, (m - 69.0) / 12.0); }
double ftom(double f) { return 12.0 * log2(f / 440.0) + 69.0; }
double dbtoa(double db) { return pow(10.0, db / 20.0); }
double atodb(double a) { return 20.0 * log10(a); }

double scale(double value, double low, double high, double Low, double High) {
  return Low + (High - Low) * ((value - low) / (high - low));
}

double uniform(double high = 1, double low = 0) {
  return low + (high - low) * double(rand()) / RAND_MAX;
}

void mono(double f) { printf("%lf\n", f); }
void stereo(double f, double v) { printf("%lf,%lf\n", f, v); }
