#pragma once

#include <gsl/span>

#include <atomic>
#include <cstdint>

constexpr const float VOLUME_0DB = (0.0f);
constexpr const float VOLUME_MUTED = (-200.0f);

class DspVolume
{

public:
    // values in dB
    
    float gain_current() const { return m_gain_current.load(); };
    void set_gain_current(float val) { m_gain_current.store(val); };
    
    float gain_desired() const { return m_gain_desired.load(); };
    void set_gain_desired(float val) { m_gain_desired.store(val); };
    
    bool muted() const { return m_muted.load(); };
    void set_muted(bool val) { m_muted.store(val); };

    bool processing() const { return m_processing.load(); };
    virtual void set_processing(bool val) { m_processing.store(val); };

    virtual void process(gsl::span<int16_t> samples, int32_t channels);
    virtual float fade_step(size_t frame_count);

protected:
    void do_process(gsl::span<int16_t> samples);

    std::atomic_bool   m_processing{false};
    const uint16_t     m_sample_rate = 48000;

private:
    std::atomic<float> m_gain_current{VOLUME_0DB};   // decibels
    std::atomic<float> m_gain_desired{VOLUME_0DB};   // decibels
    std::atomic_bool   m_muted{false};
};
