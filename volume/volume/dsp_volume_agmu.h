#pragma once

// "Make up gain / Normalize" variant

#include <QtCore/QObject>
#include "dsp_volume.h"

class DspVolumeAGMU : public DspVolume
{
    Q_OBJECT

public:
    explicit DspVolumeAGMU(QObject* parent = nullptr);

    void process(int16_t* samples, int32_t sample_count, int32_t channels);
    float GetFadeStep(int32_t sample_count);
    int16_t GetPeak() const;
    void setPeak(int16_t val);    //Overwrite peak; use for reinitializations with cache values etc.
    float computeGainDesired();

    void reset_peak() { m_peak = 0; }

private:
    const float kRateLouder = 90.0f;
    const float kRateQuieter = 120.0f;
    int16_t m_peak = 0;
};
