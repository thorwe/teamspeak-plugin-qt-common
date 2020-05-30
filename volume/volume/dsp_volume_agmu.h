#pragma once

// "Make up gain / Normalize" variant

#include "dsp_volume.h"

class DspVolumeAGMU final : public DspVolume
{

public:
    float fade_step(size_t frame_count) override;

    void process(gsl::span<int16_t> samples, int32_t channels) override;

    int16_t peak() const { return m_peak.load(); };
    void set_peak(int16_t val) { m_peak.store(val); };    //Overwrite peak; use for reinitializations with cache values etc.
    void reset_peak() { m_peak.store(0); }

private:
    const float kRateLouder = 90.0f;
    const float kRateQuieter = 120.0f;
    std::atomic_int16_t m_peak{0};
};
