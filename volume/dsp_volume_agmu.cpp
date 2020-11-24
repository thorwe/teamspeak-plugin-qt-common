#include "volume/dsp_volume_agmu.h"

#include "volume/db.h"
#include "volume/dsp_helpers.h"

#include "gsl/gsl_util"

#include <algorithm>
#include <limits>

// Funcs
namespace
{

auto compute_gain_desired(int16_t peak) -> float
{
    return std::min((lin2db(std::numeric_limits<int16_t>::max() / peak)) - 2, 12.0f);  // leave some headroom
}

}  // namespace

void DspVolumeAGMU::process(gsl::span<int16_t> samples, int32_t channels)
{
    const auto old_peak = m_peak.load();
    const auto sample_count = samples.size();
    const auto frame_count = sample_count / channels;
    auto peak = getPeak(samples.data(), gsl::narrow_cast<int32_t>(sample_count));
    peak = std::max(old_peak, peak);
    if (peak != old_peak)
    {
        m_peak.store(peak);
        set_gain_desired(compute_gain_desired(peak));
    }
    set_gain_current(fade_step(frame_count));
    do_process(samples);
}

// Compute gain change
auto DspVolumeAGMU::fade_step(size_t frame_count) -> float
{
    auto current_gain = gain_current();
    auto desired_gain = gain_desired();
    if (current_gain != desired_gain)
    {
        float fade_step_down = (kRateQuieter / m_sample_rate) * frame_count;
        float fade_step_up = (kRateLouder / m_sample_rate) * frame_count;
        if (current_gain < desired_gain - fade_step_up)
            current_gain += fade_step_up;
        else if (current_gain > desired_gain + fade_step_down)
            current_gain -= fade_step_down;
        else
            current_gain = desired_gain;

    }
    return current_gain;
}
