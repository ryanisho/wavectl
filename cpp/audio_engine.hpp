#pragma once
#include "rtaudio/RtAudio.h"
#include "biquad.hpp"
#include <atomic>
#include <memory>
#include <string>

class AudioEngine
{
public:
    AudioEngine(const std::string &in_dev, const std::string &out_dev,
                unsigned sample_rate = 48000, unsigned block_size = 256, unsigned channels = 2);
    ~AudioEngine();

    void set_output_gain_db(float db);
    void set_eq_low_db(float db);
    void set_eq_mid_db(float db);
    void set_eq_high_db(float db);
    void set_tilt_db(float db);
    void set_pan01(float p); // 0 = L, 1 = R
    void toggle_bypass();

private:
    unsigned fs, block, ch;
    std::unique_ptr<RtAudio> dac;
    int inId = -1, outId = -1;

    std::atomic<float> makeup_db{6.0f};
    std::atomic<float> eq_low{0.0f}, eq_mid{0.0f}, eq_high{0.0f}, tilt{0.0f};
    std::atomic<float> pan{0.5f};
    std::atomic<bool> bypass{false};

    Biquad lsL, lsR, pkL, pkR, hsL, hsR;

    void refreshFilters();
    static int callback(void *outBuf, void *inBuf, unsigned nFrames,
                        double streamTime, unsigned status, void *user);
};
