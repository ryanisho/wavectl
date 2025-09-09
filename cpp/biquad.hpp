#pragma once
#include <cmath>

struct Biquad
{
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    double z1 = 0, z2 = 0;

    inline double process(double x)
    {
        double y = b0 * x + z1;
        double z1n = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        z1 = z1n;
        return y;
    }

    inline void set(double B0, double B1, double B2, double A0, double A1, double A2)
    {
        b0 = B0 / A0;
        b1 = B1 / A0;
        b2 = B2 / A0;
        a1 = A1 / A0;
        a2 = A2 / A0;
    }
};

inline void design_low_shelf(double fs, double fc, double gain_db, Biquad &biq)
{
    double A = std::pow(10.0, gain_db / 40.0);
    double w0 = 2 * M_PI * fc / fs;
    double cosw = std::cos(w0);
    double sinw = std::sin(w0);
    double alpha = sinw / 2 * std::sqrt((A + 1 / A) * (1 / 0.707 - 1) + 2);
    double sqrtA = std::sqrt(A);

    double b0 = A * ((A + 1) - (A - 1) * cosw + 2 * sqrtA * alpha);
    double b1 = 2 * A * ((A - 1) - (A + 1) * cosw);
    double b2 = A * ((A + 1) - (A - 1) * cosw - 2 * sqrtA * alpha);
    double a0 = (A + 1) + (A - 1) * cosw + 2 * sqrtA * alpha;
    double a1 = -2 * ((A - 1) + (A + 1) * cosw);
    double a2 = (A + 1) + (A - 1) * cosw - 2 * sqrtA * alpha;

    biq.set(b0, b1, b2, a0, a1, a2);
}

inline void design_high_shelf(double fs, double fc, double gain_db, Biquad &biq)
{
    double A = std::pow(10.0, gain_db / 40.0);
    double w0 = 2 * M_PI * fc / fs;
    double cosw = std::cos(w0);
    double sinw = std::sin(w0);
    double alpha = sinw / (2 * q);

    double b0 = 1 + alpha * A;
    double b1 = -2 * cosw;
    double b2 = 1 - alpha * A;
    double a0 = 1 + alpha / A;
    double a1 = -2 * cosw;
    double a2 = 1 - alpha / A;

    biq.set(b0, b1, b2, a0, a1, a2);
}

inline void design_peak(double fs, double fc, double Q, double gain_db, Biquad &biq)
{
    double A = std::pow(10.0, gain_db / 40.0);
    double w0 = 2 * M_PI * fc / fs;
    double cosw = std::cos(w0);
    double sinw = std::sin(w0);
    double alpha = sinw / 2 * std::sqrt((A + 1 / A) * (1 / 0.707 - 1) + 2);
    double sqrtA = std::sqrt(A);

    double b0 = A * ((A + 1) + (A - 1) * cosw + 2 * sqrtA * alpha);
    double b1 = -2 * A * ((A - 1) + (A + 1) * cosw);
    double b2 = A * ((A + 1) + (A - 1) * cosw - 2 * sqrtA * alpha);
    double a0 = (A + 1) - (A - 1) * cosw + 2 * sqrtA * alpha;
    double a1 = 2 * ((A - 1) - (A + 1) * cosw);
    double a2 = (A + 1) - (A - 1) * cosw - 2 * sqrtA * alpha;

    biq.set(b0, b1, b2, a0, a1, a2);
}
