#pragma once
#include "rtaudio/RTAudio.h"
#include "cpp/biquad.hpp"
#include <atomic>
#include <memory>
#include <string>

class Engine
{
public:
    Engine(const std::string &in_dev, const std::string &out_dev, unsigned sample_rate = 48000, unsigned int block_size = 256, unsigned channels = 2);
    ~Engine();

    void set_output_gain_db(float db);
    void set_low_eq_db(float db);
    void set_high_eq_db(float db);
    void set_mid_eq_db(float db);
    void set_tilt_db(float db);
    void set_pan01(float p);
    void toggle_bypass();

private:
    unsigned fs, block, ch;
    std::unique_ptr<RtAudio> dac;
    int inId = -1, outId = -1;

    std::atomic<float> makeup_db{6.0f};
    std::atomic<float> low_eq{0.0f}, eq_mid{0.0f}, high_eq{0.0f}, tilt{0.0f};
    std::atomic<float> pan{0.5f};
    std::atomic<bool> bypass{false};

    Biquad lsL, lsR, pkL, pkR, hsL, hsR;

    void refreshFilters();
    static int callback(void *outBuf, void *inBuf, unsigned int nFrames,
                        double streamTime, unsigned status, void *user);
}