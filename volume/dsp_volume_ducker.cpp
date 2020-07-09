#include "volume/dsp_volume_ducker.h"

void DspVolumeDucker::set_processing(bool val)
{
    if (val)
    {
        if (gain_adjustment())
            set_gain_current(gain_desired());
        else
            set_gain_current(kVolume0dB);
    }
    else
        set_gain_current(kVolume0dB);

    m_processing.store(val);
}


// virtual funcs
float DspVolumeDucker::fade_step(size_t frame_count)
{
    // compute ducker gain
    float current_gain = gain_current();
    if (is_duck_blocked() || muted())
        current_gain = kVolume0dB;
    else
    {
        const auto gain_adjustment = m_gain_adjustment.load();
        const auto decay_rate = m_decay_rate.load();
        auto desired_gain = gain_desired();
        if (gain_adjustment && (current_gain != desired_gain))   // is attacking / adjusting
        {
            const auto attack_rate = m_attack_rate.load();
            float fade_step_down = (attack_rate / m_sample_rate) * frame_count;
            float fade_step_up = (decay_rate / m_sample_rate) * frame_count;
            if (current_gain < desired_gain - fade_step_up)
                current_gain += fade_step_up;
            else if (current_gain > desired_gain + fade_step_down)
                current_gain -= fade_step_down;
            else
                current_gain = desired_gain;
        }
        else if ((!gain_adjustment) && (current_gain != kVolume0dB))  // is releasing
        {
            float fade_step = (decay_rate / m_sample_rate) * frame_count;
            if (current_gain < kVolume0dB - fade_step)
                current_gain += fade_step;
            else if (current_gain > kVolume0dB + fade_step)
                current_gain -= fade_step;
            else
                current_gain = kVolume0dB;
        }
    }
    return current_gain;
}
