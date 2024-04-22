#pragma once

#include "dsp_volume.h"

#include "core/client_storage.h"
#include "core/definitions.h"

#include "teamspeak/public_definitions.h"
#include "teamspeak/public_definitions.h"

#include <cstdint>
#include <memory>
#include <utility>

namespace thorwe::volume
{

using namespace com::teamspeak;

template <typename T = DspVolume> class Volumes final : public Safe_Client_Storage<T>
{

  public:
    std::pair<T *, bool> add_volume(connection_id_t connection_id, client_id_t client_id)
    {
        return add_item(connection_id, client_id, std::make_unique<T>());
    }

    void onConnectStatusChanged(connection_id_t connection_id, int new_status, unsigned int /*error_number*/)
    {
        if (ConnectStatus::STATUS_DISCONNECTED == new_status)
            delete_items(connection_id);
    }
};

}  // namespace thorwe::volume
