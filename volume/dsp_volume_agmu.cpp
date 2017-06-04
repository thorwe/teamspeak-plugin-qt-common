#include "volume/dsp_volume_agmu.h"

#include <QtCore/QVarLengthArray>
#include <QtCore/qmath.h>

#include "volume/dsp_helpers.h"
#include "volume/db.h"
#include "core/ts_logging_qt.h"

DspVolumeAGMU::DspVolumeAGMU(QObject *parent)
{
    setParent(parent);
}

// Funcs

void DspVolumeAGMU::process(int16_t* samples, int32_t sample_count, int32_t channels)
{
    sample_count = sample_count * channels;
    auto peak = getPeak(samples, sample_count);
    peak = qMax(m_peak, peak);
    if (peak != m_peak)
    {
        m_peak = peak;
        setGainDesired(computeGainDesired());
    }
    setGainCurrent(GetFadeStep(sample_count));
    doProcess(samples, sample_count);
}

// Compute gain change
float DspVolumeAGMU::GetFadeStep(int sampleCount)
{
    auto current_gain = getGainCurrent();
    auto desired_gain = getGainDesired();
    if (current_gain != desired_gain)
    {
        float fade_step_down = (kRateQuieter / m_sampleRate) * sampleCount;
        float fade_step_up = (kRateLouder / m_sampleRate) * sampleCount;
        if (current_gain < desired_gain - fade_step_up)
            current_gain += fade_step_up;
        else if (current_gain > desired_gain + fade_step_down)
            current_gain -= fade_step_down;
        else
            current_gain = desired_gain;

    }
    return current_gain;
}

int16_t DspVolumeAGMU::GetPeak() const
{
    return m_peak;
}

void DspVolumeAGMU::setPeak(int16_t val)
{
    m_peak = val;
}

float DspVolumeAGMU::computeGainDesired()
{
    return qMin((lin2db(32768.f / m_peak)) -2, 12.0f); // leave some headroom
}
