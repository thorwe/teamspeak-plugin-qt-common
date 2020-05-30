#pragma once

#include "dsp_volume.h"

#include "core/definitions.h"
#include "core/client_storage.h"

#include "teamspeak/public_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"

#include <cstdint>
#include <memory>
#include <utility>

namespace thorwe {

    namespace volume {
        template<typename T = DspVolume>
        class Volumes final : public Safe_Client_Storage<T> {
        
        public:
            std::pair<T*, bool> add_volume(ts::connection_id_t connection_id, ts::client_id_t client_id) { return add_item(connection_id, client_id, std::make_unique<T>()); }

            void onConnectStatusChanged(ts::connection_id_t connection_id, int new_status, unsigned int /*error_number*/)
            {
                if (new_status == ConnectStatus::STATUS_DISCONNECTED)
                    delete_items(connection_id);
            }
        };
    }

} // namespace thorwe
