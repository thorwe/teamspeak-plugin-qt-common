#include "volume/dsp_volume.h"

#include "volume/db.h"

#include <algorithm>
#include <cmath>
#include <limits>

constexpr const float kGainFadeRate = 400.0F;  // Rate to fade at (dB per second)

void DspVolume::process(gsl::span<int16_t> samples, int32_t channels)
{
    const auto frame_count = samples.size() / channels;
    set_gain_current(fade_step(frame_count));
    do_process(samples);
}

auto DspVolume::fade_step(size_t frame_count) -> float
{
    // compute manual gain
    auto current_gain = gain_current();
    const auto desired_gain = gain_desired();
    if (muted())
    {
        float fade_step = (kGainFadeRate / m_sample_rate) * frame_count;
        if (current_gain < kVolumeMuted - fade_step)
            current_gain += fade_step;
        else if (current_gain > kVolumeMuted + fade_step)
            current_gain -= fade_step;
        else
            current_gain = kVolumeMuted;
    }
    else if (current_gain != desired_gain)
    {
        float fade_step = (kGainFadeRate / m_sample_rate) * frame_count;
        if (current_gain < desired_gain - fade_step)
            current_gain += fade_step;
        else if (current_gain > desired_gain + fade_step)
            current_gain -= fade_step;
        else
            current_gain = desired_gain;
    }
    return current_gain;
}

//! Apply volume (no need to care for channels as we're lazily not changing gain within a buffer)
void DspVolume::do_process(gsl::span<int16_t> samples)
{
    const auto mix_gain = static_cast<float>(db2lin_alt2(gain_current()));
    const auto samples_size = samples.size();
    for (auto i = decltype(samples_size){0}; i < samples_size; ++i)
    {
        samples[i] = static_cast<int16_t>(std::clamp<int32_t>(lround(samples[i] * mix_gain),
                                                              std::numeric_limits<int16_t>::min(),
                                                              std::numeric_limits<int16_t>::max()));
    }
}
