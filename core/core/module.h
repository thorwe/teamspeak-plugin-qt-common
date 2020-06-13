#pragma once

#include <atomic>

class Module
{

public:
    bool enabled() const { return m_enabled.load(); };
    bool blocked() const { return m_blocked.load(); };
    bool running() const; // result of enabled and blocked

    void set_enabled(bool value);
    void set_blocked(bool value);

protected:
    virtual void on_enabled_changed(bool) {}
    virtual void on_blocked_changed(bool) {}
    virtual void on_running_changed(bool) {}

private:
    std::atomic_bool m_enabled{ true };
    std::atomic_bool m_blocked{ false };
};
