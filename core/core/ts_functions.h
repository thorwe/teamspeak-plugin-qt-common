#pragma once

// low level wrapper for ts functions

#include "core/ts_error.h"

#include "definitions.h"
#include "plugin_definitions.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_definitions.h"

#include "gsl/span"

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace com::teamspeak::pluginsdk::funcs
{

std::tuple<std::error_code, std::string> get_clientlib_version();
std::tuple<std::error_code, uint64_t> get_clientlib_version_number();
std::string get_plugin_version(bool with_api = true);
std::tuple<std::error_code, connection_id_t> spawn_connection(int32_t port);
std::error_code destroy_connection(connection_id_t connection_id);

/* Error handling */
std::string get_error_message(int32_t error_code);

/* Memory management */
// handled internally
// std::error_code freeMemory(void* pointer);

/* Logging */
std::error_code log_message(std::string_view message,
                            LogLevel severity = LogLevel_INFO,
                            std::string_view channel = {},
                            uint64_t log_id = 0);

/* Sound */
namespace sound
{

    enum class IO : int8_t
    {
        Out = 0,
        In
    };
    std::tuple<std::error_code, std::string> get_default_mode(IO io);
    std::tuple<std::error_code, std::vector<std::string>> get_modes(IO io);
    struct Audio_Device_Ident
    {
        std::string id;
        std::string name;
    };
    std::tuple<std::error_code, Audio_Device_Ident> get_default_device(IO io);
    std::tuple<std::error_code, std::vector<Audio_Device_Ident>> get_devices(IO io,
                                                                             std::string_view mode_id = "");
    std::error_code
    open_device(connection_id_t connection_id, IO io, std::string_view mode_id, std::string_view device_id);
    std::tuple<std::error_code, std::string> get_current_mode(connection_id_t connection_id, IO io);
    std::tuple<std::error_code, std::string, bool> get_current_device_name(connection_id_t connection_id,
                                                                           IO io);
    std::error_code close_device(connection_id_t connection_id, IO io, bool graceful = true);
    std::error_code activate_capture_device(connection_id_t connection_id);
    std::tuple<std::error_code, uint64_t>
    play_wave_file_handle(connection_id_t connection_id, std::string_view path, bool loop);
    std::error_code pause_wave_file_handle(connection_id_t connection_id, uint64_t wave_handle, bool pause);
    std::error_code close_wave_file_handle(connection_id_t connection_id, uint64_t wave_handle);
    std::error_code play_wave_file(connection_id_t connection_id, std::string_view path);
    std::error_code register_custom_device(std::string_view device_id,
                                           std::string_view device_name,
                                           int32_t capture_sample_rate,
                                           int32_t capture_channels,
                                           int32_t playback_sample_rate,
                                           int32_t playback_channels);
    std::error_code unregister_custom_device(std::string_view device_id);
    std::error_code
    process_custom_capture_data(std::string_view device_name, const short *buffer, int32_t samples);
    std::error_code
    acquire_custom_playback_data(std::string_view device_name, short *buffer, int32_t samples);

    /* Preprocessor */
    std::tuple<std::error_code, float> get_preprocessor_info_value_float(connection_id_t connection_id,
                                                                         std::string_view ident);
    std::tuple<std::error_code, std::string> get_preprocessor_config_value(connection_id_t connection_id,
                                                                           std::string_view ident);
    std::error_code set_preprocessor_config_value(connection_id_t connection_id,
                                                  std::string_view ident,
                                                  std::string_view value);

    /* Encoder */
    std::tuple<std::error_code, std::string> get_encode_config_value(connection_id_t connection_id,
                                                                     std::string_view ident);

    /* Playback */
    std::tuple<std::error_code, float> get_playback_config_value_as_float(connection_id_t connection_id,
                                                                          std::string_view ident);
    std::error_code
    set_playback_config_value(connection_id_t connection_id, std::string_view ident, std::string_view value);
    std::error_code
    set_client_volume_modifier(connection_id_t connection_id, client_id_t client_id, float value);

    /* Recording */
    std::error_code set_voice_recording(connection_id_t connection_id, bool on_off);

    /* 3D sound positioning */
    std::error_code systemset_3d_listener_attributes(connection_id_t connection_id,
                                                     const TS3_VECTOR &position,
                                                     const TS3_VECTOR &forward,
                                                     const TS3_VECTOR &up);
    std::error_code
    set_3d_wave_attributes(connection_id_t connection_id, uint64_t wave_handle, const TS3_VECTOR &position);
    std::error_code
    systemset_3d_settings(connection_id_t connection_id, float distance_factor, float rolloff_scale);
    std::error_code channelset_3d_attributes(connection_id_t connection_id,
                                             client_id_t client_id,
                                             const TS3_VECTOR &position);

}  // namespace sound

/* Interaction with the server */
std::error_code start_connection(connection_id_t connection_id,
                                 std::string_view identity,
                                 std::string_view ip,
                                 uint32_t port,
                                 std::string_view nickname,
                                 gsl::span<std::string> default_channel_array,
                                 std::string_view defaultChannelPassword,
                                 std::string_view server_pw);
std::error_code stop_connection(connection_id_t connection_id, std::string_view message);
std::error_code request_client_move(connection_id_t connection_id,
                                    client_id_t client_id,
                                    channel_id_t new_channel_id,
                                    std::string_view password = "",
                                    std::string_view return_code = "");
std::error_code request_client_properties(connection_id_t connection_id,
                                          client_id_t client_id,
                                          std::string_view return_code = "");
enum class Kick_From : int8_t
{
    Server = 0,
    Channel
};
std::error_code request_client_kick(connection_id_t connection_id,
                                    client_id_t client_id,
                                    Kick_From kick_from,
                                    std::string_view reason,
                                    std::string_view return_code = "");
std::error_code request_channel_delete(connection_id_t connection_id,
                                       channel_id_t channel_id,
                                       bool force,
                                       std::string_view return_code = "");
std::error_code request_channel_move(connection_id_t connection_id,
                                     channel_id_t channel_id,
                                     channel_id_t new_channel_parent_id,
                                     channel_id_t new_channel_order,
                                     std::string_view return_code = "");
std::error_code request_send_text_msg_private(connection_id_t connection_id,
                                              std::string_view message,
                                              client_id_t target_client_id,
                                              std::string_view return_code = "");
std::error_code request_send_text_msg_channel(connection_id_t connection_id,
                                              std::string_view message,
                                              channel_id_t target_channel_id,
                                              std::string_view return_code = "");
std::error_code request_send_text_msg_server(connection_id_t connection_id,
                                             std::string_view message,
                                             std::string_view return_code = "");
std::error_code request_connection_info(connection_id_t connection_id,
                                        client_id_t client_id,
                                        std::string_view return_code = "");
std::error_code request_client_set_whisperlist(connection_id_t connection_id,
                                               client_id_t client_id,
                                               gsl::span<channel_id_t> target_channel_ids,
                                               gsl::span<client_id_t> target_client_ids,
                                               std::string_view return_code = "");
std::error_code request_channel_subscribe(connection_id_t connection_id,
                                          bool onoff,
                                          gsl::span<channel_id_t> channel_ids,
                                          std::string_view return_code = "");
std::error_code
request_channel_subscribe_all(connection_id_t connection_id, bool onoff, std::string_view return_code = "");
std::error_code request_channel_description(connection_id_t connection_id,
                                            channel_id_t channel_id,
                                            std::string_view return_code = "");
std::error_code request_mute_clients(connection_id_t connection_id,
                                     gsl::span<client_id_t> client_ids,
                                     bool mute,
                                     bool temporary,
                                     std::string_view return_code = "");
std::error_code request_client_poke(connection_id_t connection_id,
                                    client_id_t client_id,
                                    std::string_view message,
                                    std::string_view return_code = "");
std::error_code request_client_ids(connection_id_t connection_id,
                                   std::string_view client_uid,
                                   std::string_view return_code = "");
std::error_code request_client_chat_closed(connection_id_t connection_id,
                                           std::string_view client_uid,
                                           client_id_t client_id,
                                           std::string_view return_code = "");
std::error_code request_client_chat_composing(connection_id_t connection_id,
                                              client_id_t client_id,
                                              std::string_view return_code = "");
namespace server_temporary_password
{
    std::error_code request_add(connection_id_t connection_id,
                                std::string_view password,
                                std::string_view description,
                                uint64_t duration,
                                channel_id_t target_channel_id,
                                std::string_view target_channel_pw,
                                std::string_view return_code = "");

    std::error_code
    request_del(connection_id_t connection_id, std::string_view pw, std::string_view return_code = "");

    std::error_code request_list(connection_id_t connection_id, std::string_view return_code = "");
}  // namespace server_temporary_password

/* Access clientlib information */

/* Query own client ID */
std::tuple<std::error_code, client_id_t> get_client_id(connection_id_t connection_id);

/* Client info */
std::tuple<std::error_code, int32_t> get_client_self_property_as_int(connection_id_t connection_id,
                                                                     size_t flag);
std::tuple<std::error_code, std::string> get_client_self_property_as_string(connection_id_t connection_id,
                                                                            size_t flag);
std::error_code set_client_self_property_as_int(connection_id_t connection_id, size_t flag, int32_t value);
std::error_code
set_client_self_property_as_string(connection_id_t connection_id, size_t flag, std::string_view value);
std::error_code flush_client_self_updates(connection_id_t connection_id, std::string_view return_code = "");
std::tuple<std::error_code, int32_t>
get_client_property_as_int(connection_id_t connection_id, client_id_t client_id, size_t flag);
std::tuple<std::error_code, uint64_t>
get_client_property_as_uint64(connection_id_t connection_id, client_id_t client_id, size_t flag);
std::tuple<std::error_code, std::string>
get_client_property_as_string(connection_id_t connection_id, client_id_t client_id, size_t flag);
std::tuple<std::error_code, std::vector<client_id_t>> get_client_ids(connection_id_t connection_id);
std::tuple<std::error_code, channel_id_t> get_channel_of_client(connection_id_t connection_id,
                                                                client_id_t client_id);

/* Channel info */
std::tuple<std::error_code, int32_t>
get_channel_property_as_int(connection_id_t connection_id, channel_id_t channel_id, size_t flag);
std::tuple<std::error_code, uint64_t>
get_channel_property_as_uint64(connection_id_t connection_id, channel_id_t channel_id, size_t flag);
std::tuple<std::error_code, std::string>
get_channel_property_as_string(connection_id_t connection_id, channel_id_t channel_id, size_t flag);
std::tuple<std::error_code, channel_id_t>
get_channel_id_from_channel_names(connection_id_t connection_id, gsl::span<std::string> channel_names);
std::error_code set_channel_variable_as_int(connection_id_t connection_id,
                                            channel_id_t channel_id,
                                            size_t flag,
                                            int32_t value);
std::error_code set_channel_variable_as_uint64(connection_id_t connection_id,
                                               channel_id_t channel_id,
                                               size_t flag,
                                               uint64_t value);
std::error_code set_channel_variable_as_string(connection_id_t connection_id,
                                               channel_id_t channel_id,
                                               size_t flag,
                                               std::string_view value);
std::error_code flush_channel_updates(connection_id_t connection_id,
                                      channel_id_t channel_id,
                                      std::string_view return_code = "");
std::error_code flush_channel_creation(connection_id_t connection_id,
                                       channel_id_t channel_parent_id,
                                       std::string_view return_code = "");
std::tuple<std::error_code, std::vector<channel_id_t>> get_channel_ids(connection_id_t connection_id);
std::tuple<std::error_code, channel_id_t> get_channel_id(connection_id_t connection_id);
std::tuple<std::error_code, std::vector<client_id_t>> get_channel_client_ids(connection_id_t connection_id,
                                                                             channel_id_t channel_id);
std::tuple<std::error_code, channel_id_t> get_parent_channel_of_channel(connection_id_t connection_id,
                                                                        channel_id_t channel_id);

/* Server info */
std::tuple<std::error_code, std::vector<connection_id_t>> get_server_connection_handler_ids();
std::tuple<std::error_code, int32_t> get_server_property_as_int(connection_id_t connection_id, size_t flag);
std::tuple<std::error_code, uint64_t> get_server_property_as_uint64(connection_id_t connection_id,
                                                                    size_t flag);
std::tuple<std::error_code, std::string> get_server_property_as_string(connection_id_t connection_id,
                                                                       size_t flag);
std::error_code request_server_properties(connection_id_t connection_id);

/* Connection info */
std::tuple<std::error_code, int> get_connection_status(connection_id_t connection_id);
std::tuple<std::error_code, uint64_t>
get_connection_property_as_uint64(connection_id_t connection_id, client_id_t client_id, size_t flag);
std::tuple<std::error_code, double>
get_connection_property_as_double(connection_id_t connection_id, client_id_t client_id, size_t flag);
std::tuple<std::error_code, std::string>
get_connection_property_as_string(connection_id_t connection_id, client_id_t client_id, size_t flag);
std::error_code clean_up_connection_info(connection_id_t connection_id, client_id_t client_id);

/* Client related */
std::error_code request_client_db_id_from_uid(connection_id_t connection_id,
                                              std::string_view client_uid,
                                              std::string_view return_code = "");
std::error_code request_client_name_from_uid(connection_id_t connection_id,
                                             std::string_view client_uid,
                                             std::string_view return_code = "");
std::error_code request_client_name_from_dbid(connection_id_t connection_id,
                                              client_db_id_t client_db_id,
                                              std::string_view return_code = "");
std::error_code request_client_edit_description(connection_id_t connection_id,
                                                client_id_t client_id,
                                                std::string_view client_description,
                                                std::string_view return_code = "");
std::error_code request_client_set_is_talker(connection_id_t connection_id,
                                             client_id_t client_id,
                                             bool is_talker,
                                             std::string_view return_code = "");
std::error_code request_is_talker(connection_id_t connection_id,
                                  bool is_talker_request,
                                  std::string_view message,
                                  std::string_view return_code = "");

/* Plugin related */
std::error_code request_send_client_query_command(connection_id_t connection_id,
                                                  std::string_view command,
                                                  std::string_view return_code = "");

namespace file_transfer
{

    std::tuple<std::error_code, std::string> get_transfer_file_name(transfer_id_t transfer_id);
    std::tuple<std::error_code, std::string> get_transfer_file_path(transfer_id_t transfer_id);
    std::tuple<std::error_code, uint64_t> get_transfer_file_size(transfer_id_t transfer_id);
    std::tuple<std::error_code, uint64_t> get_transfer_file_size_done(transfer_id_t transfer_id);
    std::tuple<std::error_code, bool>
    is_transfer_sender(transfer_id_t transfer_id);  // true == upload, false == download
    std::tuple<std::error_code, int32_t> get_transfer_status(transfer_id_t transfer_id);
    std::tuple<std::error_code, float> get_transfer_speed_current(transfer_id_t transfer_id);
    std::tuple<std::error_code, float> get_transfer_speed_average(transfer_id_t transfer_id);
    std::tuple<std::error_code, uint64_t> get_transfer_run_time(transfer_id_t transfer_id);
    std::tuple<std::error_code, transfer_id_t> send_file(connection_id_t connection_id,
                                                         channel_id_t channel_id,
                                                         std::string_view channel_pw,
                                                         std::string_view file,
                                                         bool overwrite,
                                                         bool resume,
                                                         std::string_view source_directory,
                                                         std::string_view return_code = "");
    std::tuple<std::error_code, transfer_id_t> request_file(connection_id_t connection_id,
                                                            channel_id_t channel_id,
                                                            std::string_view channel_pw,
                                                            std::string_view file,
                                                            bool overwrite,
                                                            bool resume,
                                                            std::string_view destination_directory,
                                                            std::string_view return_code = "");
    std::error_code halt_transfer(connection_id_t connection_id,
                                  transfer_id_t transfer_id,
                                  bool delete_unfinished_file,
                                  std::string_view return_code = "");
    std::error_code request_file_list(connection_id_t connection_id,
                                      channel_id_t channel_id,
                                      std::string_view channel_pw,
                                      std::string_view path,
                                      std::string_view return_code = "");
    std::error_code request_file_info(connection_id_t connection_id,
                                      channel_id_t channel_id,
                                      std::string_view channel_pw,
                                      std::string_view file,
                                      std::string_view return_code = "");
    std::error_code request_delete_files(connection_id_t connection_id,
                                         channel_id_t channel_id,
                                         std::string_view channel_pw,
                                         gsl::span<std::string> full_file_paths,
                                         std::string_view return_code = "");
    std::error_code request_create_directory(connection_id_t connection_id,
                                             channel_id_t channel_id,
                                             std::string_view channel_pw,
                                             std::string_view directoryPath,
                                             std::string_view return_code = "");
    std::error_code request_rename_file(connection_id_t connection_id,
                                        channel_id_t from_channel_id,
                                        std::string_view channel_pw,
                                        channel_id_t to_channel_id,
                                        std::string_view to_channel_pw,
                                        std::string_view old_file,
                                        std::string_view new_file,
                                        std::string_view return_code = "");

}  // namespace file_transfer

namespace offline_messages
{

    std::error_code request_add(connection_id_t connection_id,
                                std::string_view to_client_uid,
                                std::string_view subject,
                                std::string_view message,
                                std::string_view return_code = "");
    std::error_code
    request_del(connection_id_t connection_id, uint64_t message_id, std::string_view return_code = "");
    std::error_code
    request_get(connection_id_t connection_id, uint64_t message_id, std::string_view return_code = "");
    std::error_code request_list(connection_id_t connection_id, std::string_view return_code = "");
    std::error_code request_update_flag(connection_id_t connection_id,
                                        uint64_t message_id,
                                        int32_t flag,
                                        std::string_view return_code = "");

}  // namespace offline_messages

/* Interacting with the server - confirming passwords */
std::error_code verify_password_server(connection_id_t connection_id,
                                       std::string_view server_pw,
                                       std::string_view return_code = "");
std::error_code verify_password_channel(connection_id_t connection_id,
                                        channel_id_t channel_id,
                                        std::string_view channel_pw,
                                        std::string_view return_code = "");

namespace ban
{
    std::error_code ban_client(connection_id_t connection_id,
                               client_id_t client_id,
                               uint64_t time_in_seconds,
                               std::string_view reason,
                               std::string_view return_code = "");
    std::error_code ban_add(connection_id_t connection_id,
                            std::string_view ip_regexp,
                            std::string_view name_regexp,
                            std::string_view target_client_uid,
                            std::string_view target_client_mytsid,
                            uint64_t time_in_seconds,
                            std::string_view reason,
                            std::string_view return_code = "");
    std::error_code ban_client_db_id(connection_id_t connection_id,
                                     client_db_id_t client_db_id,
                                     uint64_t timeInSeconds,
                                     std::string_view reason,
                                     std::string_view return_code = "");
    std::error_code
    ban_del(connection_id_t connection_id, uint64_t ban_id, std::string_view return_code = "");
    std::error_code ban_del_all(connection_id_t connection_id, std::string_view return_code = "");
    std::error_code request_list(connection_id_t connection_id,
                                 uint64_t start,
                                 uint32_t duration,
                                 std::string_view return_code = "");
}  // namespace ban

namespace complain
{

    std::error_code request_add(connection_id_t connection_id,
                                client_db_id_t targetClientDatabaseID,
                                std::string_view reason,
                                std::string_view return_code = "");
    std::error_code request_del(connection_id_t connection_id,
                                client_db_id_t targetClientDatabaseID,
                                client_db_id_t fromClientDatabaseID,
                                std::string_view return_code = "");
    std::error_code request_del_all(connection_id_t connection_id,
                                    client_db_id_t targetClientDatabaseID,
                                    std::string_view return_code = "");
    std::error_code request_list(connection_id_t connection_id,
                                 client_db_id_t targetClientDatabaseID,
                                 std::string_view return_code = "");
}  // namespace complain

namespace permissions
{

    // valid for Group_Category::Server & Group_Category::Channel
    std::tuple<std::error_code, std::string> get_permission_name(connection_id_t connection_id,
                                                                 permission_id_t permission_id);
    std::error_code request_group_list(connection_id_t connection_id,
                                       Group_List_Type group_list_type,
                                       std::string_view return_code = "");
    std::error_code request_group_add(connection_id_t connection_id,
                                      Group_List_Type group_list_type,
                                      std::string_view group_name,
                                      int32_t group_type,
                                      std::string_view return_code = "");
    std::error_code request_group_del(connection_id_t connection_id,
                                      Group_List_Type group_list_type,
                                      uint64_t group_id,
                                      bool force,
                                      std::string_view return_code = "");

    namespace server_group
    {
        struct Permission_Entry
        {
            permission_id_t id = 0;
            int32_t value = 0;
            bool negated = false;
            bool skip = false;
        };

        std::error_code request_change_client(connection_id_t connection_id,
                                              bool add_remove,
                                              uint64_t group_id,
                                              client_db_id_t client_db_id,
                                              std::string_view return_code = "");
        std::error_code request_groups_by_client_id(connection_id_t connection_id,
                                                    client_db_id_t client_db_id,
                                                    std::string_view return_code = "");
        std::error_code request_add_perm(connection_id_t connection_id,
                                         permission_group_id_t server_group_id,
                                         bool continue_on_error,
                                         gsl::span<Permission_Entry> entries,
                                         std::string_view return_code = "");
        std::error_code request_del_perm(connection_id_t connection_id,
                                         permission_group_id_t server_group_id,
                                         bool continue_on_error,
                                         gsl::span<permission_id_t> ids,
                                         std::string_view return_code = "");
        std::error_code request_list(connection_id_t connection_id,
                                     permission_group_id_t server_group_id,
                                     std::string_view return_code = "");
        std::error_code request_client_list(connection_id_t connection_id,
                                            permission_group_id_t group_id,
                                            bool with_names,
                                            std::string_view return_code = "");
    }  // namespace server_group

    namespace channel_group
    {
        struct Permission_Entry
        {
            permission_id_t id = 0;
            int32_t value = 0;
        };

        std::error_code request_add_perm(connection_id_t connection_id,
                                         permission_group_id_t group_id,
                                         bool continue_on_error,
                                         gsl::span<Permission_Entry> entries,
                                         std::string_view return_code = "");
        std::error_code request_del_perm(connection_id_t connection_id,
                                         permission_group_id_t group_id,
                                         bool continue_on_error,
                                         gsl::span<permission_id_t> ids,
                                         std::string_view return_code = "");
        std::error_code request_list(connection_id_t connection_id,
                                     permission_group_id_t group_id,
                                     std::string_view return_code = "");
        struct Set_Client_Channel_Group_Entry
        {
            permission_group_id_t channel_group_id = 0;
            channel_id_t channel_id = 0;
            client_db_id_t client_db_id = 0;
        };

        std::error_code request_set_client(connection_id_t connection_id,
                                           gsl::span<Set_Client_Channel_Group_Entry> entries,
                                           std::string_view return_code = "");
    }  // namespace channel_group

    namespace channel
    {
        struct Permission_Entry
        {
            permission_id_t id = 0;
            int32_t value = 0;
        };

        std::error_code request_add_perm(connection_id_t connection_id,
                                         channel_id_t channel_id,
                                         gsl::span<Permission_Entry> entries,
                                         std::string_view return_code = "");
        std::error_code request_del_perm(connection_id_t connection_id,
                                         channel_id_t channel_id,
                                         gsl::span<permission_id_t> ids,
                                         std::string_view return_code = "");
        std::error_code request_list(connection_id_t connection_id,
                                     channel_id_t channel_id,
                                     std::string_view return_code = "");
    }  // namespace channel

    namespace client
    {
        struct Permission_Entry
        {
            permission_id_t id = 0;
            int32_t value = 0;
            bool skip = false;
        };

        std::error_code request_add_perm(connection_id_t connection_id,
                                         client_db_id_t client_db_id,
                                         gsl::span<Permission_Entry> entries,
                                         std::string_view return_code = "");
        std::error_code request_del_perm(connection_id_t connection_id,
                                         client_db_id_t client_db_id,
                                         gsl::span<permission_id_t> ids,
                                         std::string_view return_code = "");
        std::error_code request_list(connection_id_t connection_id,
                                     client_db_id_t client_db_id,
                                     std::string_view return_code = "");
    }  // namespace client

    namespace channel_client
    {
        struct Permission_Entry
        {
            permission_id_t id = 0;
            int32_t value = 0;
        };

        std::error_code request_add_perm(connection_id_t connection_id,
                                         channel_id_t channel_id,
                                         client_db_id_t client_db_id,
                                         gsl::span<Permission_Entry> entries,
                                         std::string_view return_code = "");
        std::error_code request_del_perm(connection_id_t connection_id,
                                         channel_id_t channel_id,
                                         client_db_id_t client_db_id,
                                         gsl::span<permission_id_t> ids,
                                         std::string_view return_code = "");
        std::error_code request_list(connection_id_t connection_id,
                                     channel_id_t channel_id,
                                     client_db_id_t client_db_id,
                                     std::string_view return_code = "");
    }  // namespace channel_client

    std::error_code privilege_key_use(connection_id_t connection_id,
                                      std::string_view token,
                                      std::string_view return_code = "");
    std::error_code request_permission_list(connection_id_t connection_id, std::string_view return_code = "");
    std::error_code request_permission_overview(connection_id_t connection_id,
                                                client_db_id_t client_db_id,
                                                channel_id_t channel_id,
                                                std::string_view return_code = "");
}  // namespace permissions

/* Client functions */
std::string get_app_path();
std::string get_resources_path();
std::string get_config_path();
std::string get_plugin_path(std::string_view plugin_id);
connection_id_t get_current_server_connection_handler_id();
void print_message(connection_id_t connection_id,
                   std::string_view message,
                   PluginMessageTarget message_target);
void print_message_to_current_tab(std::string_view message);
std::string urls_to_bb(std::string_view text, size_t max_len);

void send_plugin_command(connection_id_t connection_id,
                         std::string_view plugin_id,
                         std::string_view command,
                         int target_mode,
                         gsl::span<const client_id_t> target_ids,
                         std::string_view return_code = "");

std::string get_directories(std::string_view path, size_t max_len);

std::tuple<std::error_code, std::string, uint16_t, std::string>
get_server_connect_info(connection_id_t connection_id);

std::tuple<std::error_code, std::string, std::string> get_channel_connect_info(connection_id_t connection_id,
                                                                               channel_id_t channel_id);

std::string create_return_code(std::string_view plugin_id);

std::error_code
request_info_update(connection_id_t connection_id, PluginItemType item_type, uint64_t item_id);
uint64_t get_server_version(connection_id_t connection_id);
std::tuple<std::error_code, bool> is_whispering(connection_id_t connection_id, client_id_t client_id);
std::tuple<std::error_code, bool> is_receiving_whisper(connection_id_t connection_id, client_id_t client_id);

// database_empty_result is not an error, the client simply has no avatar set
std::tuple<std::error_code, std::string> get_avatar(connection_id_t connection_id, client_id_t client_id);

void set_plugin_menu_enabled(std::string_view plugin_id, int32_t menu_id, bool enabled);
void show_hotkey_setup();
void request_hotkey_input_dialog(std::string_view plugin_id,
                                 std::string_view keyword,
                                 bool is_down,
                                 void *q_parent_window);

std::tuple<std::error_code, std::vector<std::string>> get_hotkey_from_keyword(
std::string_view plugin_id, gsl::span<std::string_view> keywords, int32_t max_results);

std::tuple<std::error_code, std::string> get_client_display_name(connection_id_t connection_id,
                                                                 client_id_t client_id);

std::tuple<std::error_code, PluginBookmarkList *> get_bookmark_list();
void free_bookmark_list(PluginBookmarkList *);

std::tuple<std::error_code, int, std::vector<std::string>> get_profile_list(PluginGuiProfile profile);

std::tuple<std::error_code, connection_id_t> gui_connect(PluginConnectTab connect_tab,
                                                         std::string_view server_label,
                                                         std::string_view server_address,
                                                         std::string_view server_pw,
                                                         std::string_view nickname,
                                                         std::string_view channel,
                                                         std::string_view channel_pw,
                                                         std::string_view capture_profile,
                                                         std::string_view playback_profile,
                                                         std::string_view hotkey_profile,
                                                         std::string_view sound_profile,
                                                         std::string_view user_identity,
                                                         std::string_view one_time_key,
                                                         std::string_view phonetic_name);

std::tuple<std::error_code, connection_id_t> gui_connect_bookmark(PluginConnectTab connect_tab,
                                                                  std::string_view bookmark_uuid);

std::error_code create_bookmark(std::string_view bookmark_uuid,
                                std::string_view server_label,
                                std::string_view server_address,
                                std::string_view server_pw,
                                std::string_view nickname,
                                std::string_view channel,
                                std::string_view channel_pw,
                                std::string_view capture_profile,
                                std::string_view playback_profile,
                                std::string_view hotkey_profile,
                                std::string_view sound_profile,
                                std::string_view client_uid,
                                std::string_view one_time_key,
                                std::string_view phonetic_name);

std::tuple<std::error_code, uint32_t> get_permission_id_by_name(connection_id_t connection_id,
                                                                std::string_view permission_name);
std::tuple<std::error_code, int32_t> get_client_needed_permission(connection_id_t connection_id,
                                                                  std::string_view permission_name);
void notify_key_event(std::string_view plugin_id, std::string_view key_identifier, bool up_down);

/* Recording */
/*
 * noFileSelector true, path given: use default path
 * noFileSelector false, path given: show file selector
 * noFileSelector true, path not given: create path if not exist
 * noFileSelector false, path not given: show file selector if path not exist
 */
std::error_code
start_recording(connection_id_t connection_id, bool multitrack, bool noFileSelector, std::string_view path);
std::error_code stop_recording(connection_id_t connection_id);

/* Convenience functions */
std::error_code request_clients_move(connection_id_t connection_id,
                                     gsl::span<client_id_t> target_client_ids,
                                     channel_id_t new_channel_id,
                                     std::string_view password,
                                     std::string_view return_code = "");
std::error_code request_clients_kick(connection_id_t connection_id,
                                     gsl::span<client_id_t> target_client_ids,
                                     Kick_From kick_from,
                                     std::string_view reason,
                                     std::string_view return_code = "");

/* Helpers */
std::tuple<std::error_code, std::string> get_client_property_name(size_t flag);
std::tuple<std::error_code, std::string> get_channel_property_name(size_t flag);
std::tuple<std::error_code, std::string> get_server_property_name(size_t flag);
std::tuple<std::error_code, size_t> get_client_property_flag(std::string_view name);
std::tuple<std::error_code, size_t> get_channel_property_flag(std::string_view name);
std::tuple<std::error_code, size_t> get_server_property_flag(std::string_view name);

}  // namespace com::teamspeak::pluginsdk::funcs
