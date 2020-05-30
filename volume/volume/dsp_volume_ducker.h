#pragma once

#include "dsp_volume.h"

#include <atomic>

class DspVolumeDucker : public DspVolume
{

public:
    void set_processing(bool val) override;
    float fade_step(int sample_count) override;

    float attack_rate() const { return m_attack_rate.load(); };
    void set_attack_rate(float val) { m_attack_rate.store(val); };

    float decay_rate() const { return m_decay_rate.load(); };
    void set_decay_rate(float val) { m_decay_rate.store(val); };

    bool gain_adjustment() const { return m_gain_adjustment.load(); };
    void set_gain_adjustment(bool val) { m_gain_adjustment.store(val); };

    bool is_duck_blocked() const { return m_is_duck_blocked.load(); }; // is Whispering Blacklisting
    void set_duck_blocked(bool val) { m_is_duck_blocked.store(val); };

private:
    std::atomic<float> m_attack_rate{120.0f};
    std::atomic<float> m_decay_rate ={90.0f};

    std::atomic_bool m_gain_adjustment{false};
    std::atomic_bool m_is_duck_blocked{false};
};
