#pragma once

#include <cstdint>

namespace com
{
namespace teamspeak
{

    using connection_id_t = uint64_t;
    using channel_id_t = uint64_t;
    using client_id_t = uint16_t;
    using permission_id_t = uint32_t;
    using permission_group_id_t = uint64_t;
    using client_db_id_t = uint64_t;
    using transfer_id_t = uint16_t;

    enum class Group_Category : uint8_t
    {
        Server,
        Channel,
        Client,
        ChannelClient
    };

    enum Group_List_Type
    {
        Server,
        Channel
    };

}  // namespace teamspeak
}  // namespace com
