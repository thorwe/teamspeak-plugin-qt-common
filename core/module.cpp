#include "core/module.h"

bool Module::running() const
{
    return (m_enabled.load() && !m_blocked.load());
}


void Module::set_enabled(bool value)
{
    if (value != m_enabled.load())
    {
        const auto running_old = running();
        m_enabled.store(value);
        on_enabled_changed(value);
        if (running_old != running())
            on_running_changed(running());
    }
}

void Module::set_blocked(bool value)
{
    if (value != m_blocked.load())
    {
        const auto running_old = running();
        m_blocked.store(value);
        on_blocked_changed(value);
        if (running_old != running())
            on_running_changed(running());
    }
}
