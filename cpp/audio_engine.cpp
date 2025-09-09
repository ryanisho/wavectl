#include "audio_engine.hpp"
#include <pybind11/pybind11.h>
#include <cstring>
#include <cmath>
#include <stdexcept>

namespace py = pybind11;

AudioEngine::AudioEngine(const std::string &in_dev, const std::string &out_dev,
                         unsigned sample_rate, unsigned block_size, unsigned channels)
    : fs(sample_rate), block(block_size), ch(channels)
{
    refreshFilters();
    dac = std::make_unique<RtAudio>();

    // Find device indices
    auto findDev = [&](const std::string &name, bool wantInput)
    {
        unsigned N = dac->getDeviceCount();
        for (unsigned i = 0; i < N; i++)
        {
            auto di = dac->getDeviceInfo(i);
            if ((wantInput && di.inputChannels == 0) || (!wantInput && di.outputChannels == 0))
                continue;
            std::string low = di.name;
            for (auto &c : low)
                c = std::tolower(c);
            std::string needle = name;
            for (auto &c : needle)
                c = std::tolower(c);
            if (low.find(needle) != std::string::npos)
                return (int)i;
        }
        return -1;
    };
    inId = findDev(in_dev, true);
    outId = findDev(out_dev, false);
    if (inId < 0 || outId < 0)
        throw std::runtime_error("Device not found");

    RtAudio::StreamParameters inParams{(unsigned)inId, 0, ch};
    RtAudio::StreamParameters outParams{(unsigned)outId, 0, ch};
    RtAudio::StreamOptions opts;
    opts.flags = RTAUDIO_SCHEDULE_REALTIME | RTAUDIO_MINIMIZE_LATENCY;

    dac->openStream(&outParams, &inParams, RTAUDIO_FLOAT32, fs, &block, &AudioEngine::callback, this, &opts);
    dac->startStream();
}

AudioEngine::~AudioEngine()
{
    if (dac && dac->isStreamOpen())
    {
        dac->stopStream();
        dac->closeStream();
    }
}

int AudioEngine::callback(void *outBuf, void *inBuf, unsigned nFrames, double, unsigned, void *user)
{
    auto *self = reinterpret_cast<AudioEngine *>(user);
    float *in = reinterpret_cast<float *>(inBuf);
    float *out = reinterpret_cast<float *>(outBuf);
    if (!in || !out)
    {
        if (out)
            memset(out, 0, nFrames * self->ch * sizeof(float));
        return 0;
    }

    if (self->bypass.load())
    {
        memcpy(out, in, nFrames * self->ch * sizeof(float));
        return 0;
    }

    float g = std::pow(10.0f, self->makeup_db.load() / 20.0f);
    float lmul = std::cos(self->pan.load() * M_PI_2);
    float rmul = std::sin(self->pan.load() * M_PI_2);

    for (unsigned i = 0; i < nFrames; i++)
    {
        float L = in[self->ch * i + 0];
        float R = (self->ch > 1) ? in[self->ch * i + 1] : L;

        L = self->lsL.process(L);
        R = self->lsR.process(R);
        L = self->pkL.process(L);
        R = self->pkR.process(R);
        L = self->hsL.process(L);
        R = self->hsR.process(R);

        L = std::tanh(L * g * 1.5f) / std::tanh(1.5f);
        R = std::tanh(R * g * 1.5f) / std::tanh(1.5f);

        out[self->ch * i + 0] = L * lmul;
        if (self->ch > 1)
            out[self->ch * i + 1] = R * rmul;
    }
    return 0;
}

void AudioEngine::refreshFilters()
{
    float hs = std::clamp(eq_high.load() + tilt.load(), -12.0f, 12.0f);
    float ls = std::clamp(eq_low.load() - tilt.load(), -12.0f, 12.0f);
    double B0, B1, B2, A0, A1, A2;
    design_low_shelf(fs, 120.0, ls, lsL);
    design_low_shelf(fs, 120.0, ls, lsR);
    design_peak(fs, 1000.0, 1.0, eq_mid.load(), pkL);
    design_peak(fs, 1000.0, 1.0, eq_mid.load(), pkR);
    design_high_shelf(fs, 6000.0, hs, hsL);
    design_high_shelf(fs, 6000.0, hs, hsR);
}

// Setters
void AudioEngine::set_output_gain_db(float db) { makeup_db = std::clamp(db, -12.0f, 18.0f); }
void AudioEngine::set_eq_low_db(float db)
{
    eq_low = std::clamp(db, -12.0f, 12.0f);
    refreshFilters();
}
void AudioEngine::set_eq_mid_db(float db)
{
    eq_mid = std::clamp(db, -12.0f, 12.0f);
    refreshFilters();
}
void AudioEngine::set_eq_high_db(float db)
{
    eq_high = std::clamp(db, -12.0f, 12.0f);
    refreshFilters();
}
void AudioEngine::set_tilt_db(float db)
{
    tilt = std::clamp(db, -6.0f, 6.0f);
    refreshFilters();
}
void AudioEngine::set_pan01(float p) { pan = std::clamp(p, 0.0f, 1.0f); }
void AudioEngine::toggle_bypass() { bypass = !bypass.load(); }

// pybind11
PYBIND11_MODULE(audio_engine, m)
{
    py::class_<AudioEngine>(m, "AudioEngine")
        .def(py::init<const std::string &, const std::string &, unsigned, unsigned, unsigned>(),
             py::arg("in_dev"), py::arg("out_dev"), py::arg("fs") = 48000, py::arg("block") = 256, py::arg("ch") = 2)
        .def("set_output_gain_db", &AudioEngine::set_output_gain_db)
        .def("set_eq_low_db", &AudioEngine::set_eq_low_db)
        .def("set_eq_mid_db", &AudioEngine::set_eq_mid_db)
        .def("set_eq_high_db", &AudioEngine::set_eq_high_db)
        .def("set_tilt_db", &AudioEngine::set_tilt_db)
        .def("set_pan01", &AudioEngine::set_pan01)
        .def("toggle_bypass", &AudioEngine::toggle_bypass);
}
