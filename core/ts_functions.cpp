#include "core/ts_functions.h"

#include "plugin.h"
#include "ts3_functions.h"

#include "teamspeak/public_definitions.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"

#include "gsl/gsl_util"

#include <array>
#include <string>
#include <vector>

using namespace com::teamspeak;

static struct TS3Functions ts_funcs;
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs)
{
    ts_funcs = funcs;
}

constexpr const size_t kPermBufferSize = 128;
constexpr const size_t kPathBufferSize = 1024;
constexpr const int32_t kDisplayNameBufferSize = 512;
constexpr const int32_t kServerInfoBufferSize = 256;
constexpr const int32_t kChannelInfoBufferSize = 512;
constexpr const int32_t kReturnCodeBufferSize = 128;
constexpr const int32_t kHotkeyBufferSize = 128;

namespace com::teamspeak::pluginsdk::funcs
{


/* Versioning */
auto get_clientlib_version() -> std::tuple<std::error_code, std::string>
{
    std::string result;
    char *result_c = nullptr;
    const auto error = to_ts_errc(ts_funcs.getClientLibVersion(&result_c));
    if (ts_errc::ok == error)
    {
        result = std::string(result_c);
        ts_funcs.freeMemory(result_c);
    }
    return {error, result};
}

auto get_clientlib_version_number() -> std::tuple<std::error_code, uint64_t>
{
    uint64_t result = 0;
    const auto error = to_ts_errc(ts_funcs.getClientLibVersionNumber(&result));
    return {error, result};
}

auto get_plugin_version(bool with_api) -> std::string
{
    auto ver = std::string(ts3plugin_version());
    auto api = std::to_string(ts3plugin_apiVersion());
    return ver + (with_api ? " (API " + api + ")" : "");
}

auto spawn_connection(int32_t port) -> std::tuple<std::error_code, connection_id_t>
{
    auto result = connection_id_t{0};
    const auto error = to_ts_errc(ts_funcs.spawnNewServerConnectionHandler(port, &result));
    return {error, result};
}

auto destroy_connection(connection_id_t connection_id) -> std::error_code
{
    return to_ts_errc(ts_funcs.destroyServerConnectionHandler(connection_id));
}

/* Error handling */
auto get_error_message(int32_t error_code) -> std::string
{
    std::string result;
    char *result_c = nullptr;
    if (const auto error = to_ts_errc(ts_funcs.getErrorMessage(error_code, &result_c)); ts_errc::ok == error)
    {
        result = std::string(result_c);
        ts_funcs.freeMemory(result_c);
    }
    else
        result = "Couldn't get error message.";

    return result;
}

/* Logging */
auto log_message(std::string_view message,
                 LogLevel severity /*= LogLevel_INFO*/,
                 std::string_view channel /*= {}*/,
                 uint64_t log_id /*= 0*/) -> std::error_code
{
    if (message.empty())
        return ts_errc::parameter_invalid;

    return to_ts_errc(ts_funcs.logMessage(message.data(), severity,
                                          channel.empty() ? ts3plugin_name() : channel.data(), log_id));
}

/* Sound */
namespace sound
{

    auto get_default_mode(IO io) -> std::tuple<std::error_code, std::string>
    {
        auto error = ts_errc::parameter_invalid;
        char *result_c = nullptr;
        if (IO::Out == io)
            error = to_ts_errc(ts_funcs.getDefaultPlayBackMode(&result_c));
        else if (IO::In == io)
            error = to_ts_errc(ts_funcs.getDefaultCaptureMode(&result_c));

        const auto result = std::string{result_c};
        ts_funcs.freeMemory(result_c);
        return {error, result};
    }

    auto get_modes(IO io) -> std::tuple<std::error_code, std::vector<std::string>>
    {
        auto error = ts_errc::parameter_invalid;

        char **result_c = nullptr;
        if (IO::Out == io)
            error = to_ts_errc(ts_funcs.getPlaybackModeList(&result_c));
        else if (IO::In == io)
            error = to_ts_errc(ts_funcs.getCaptureModeList(&result_c));

        std::vector<std::string> result;
        if (ts_errc::ok == error)
        {
            for (int i = 0; result_c[i]; ++i)
            {
                result.emplace_back(result_c[i]);
                ts_funcs.freeMemory(result_c[i]);  // Free C-string
            }
            ts_funcs.freeMemory(result_c);  // Free the array
        }
        return {error, result};
    }

    auto getDefaultDevice(IO io) -> std::tuple<std::error_code, Audio_Device_Ident>
    {
        Audio_Device_Ident ident;
        const auto [error_default_mode, default_mode] = get_default_mode(io);
        if (ts_errc::ok != error_default_mode)
            return {error_default_mode, ident};

        auto error = ts_errc::parameter_invalid;
        char **result_c = nullptr;
        if (IO::Out == io)
            error = to_ts_errc(ts_funcs.getDefaultPlaybackDevice(default_mode.data(), &result_c));
        else if (IO::In == io)
            error = to_ts_errc(ts_funcs.getDefaultCaptureDevice(default_mode.data(), &result_c));

        if (ts_errc::ok == error)
        {
            ident.name = result_c[0]; /* First element: Device name */
            ident.id = result_c[1];   /* Second element: Device ID */

            /* Release the two array elements and the array */
            ts_funcs.freeMemory(result_c[0]);
            ts_funcs.freeMemory(result_c[1]);
            ts_funcs.freeMemory(result_c);
        }

        return {error, ident};
    }

    auto get_devices(IO io, std::string_view mode_id /*=""*/)
    -> std::tuple<std::error_code, std::vector<Audio_Device_Ident>>
    {
        auto mode_id_ = std::string{mode_id};
        if (mode_id.empty())
        {
            const auto [error_default_mode, default_mode] = get_default_mode(io);
            if (ts_errc::ok != error_default_mode)
                return {error_default_mode, {}};

            mode_id_ = default_mode;
        }

        char ***result_c = nullptr;
        auto error = ts_errc::parameter_invalid;
        if (IO::Out == io)
            error = to_ts_errc(ts_funcs.getPlaybackDeviceList(mode_id.data(), &result_c));
        else if (IO::In == io)
            error = to_ts_errc(ts_funcs.getCaptureDeviceList(mode_id.data(), &result_c));

        std::vector<Audio_Device_Ident> result;
        if (ts_errc::ok == error)
        {
            for (int i = 0; result_c[i]; ++i)
            {
                Audio_Device_Ident ident;
                ident.name = result_c[i][0]; /* First element: Device name */
                ident.id = result_c[i][1];   /* Second element: Device ID */
                result.push_back(std::move(ident));

                /* Free element */
                ts_funcs.freeMemory(result_c[i][0]);
                ts_funcs.freeMemory(result_c[i][1]);
                ts_funcs.freeMemory(result_c[i]);
            }
            ts_funcs.freeMemory(result_c); /* Free complete array */
        }

        return {error, result};
    }

    auto
    open_device(connection_id_t connection_id, IO io, std::string_view mode_id, std::string_view device_id)
    -> std::error_code
    {
        auto error = ts_errc::parameter_invalid;
        if (IO::Out == io)
            error = to_ts_errc(ts_funcs.openPlaybackDevice(connection_id, mode_id.data(), device_id.data()));
        else if (IO::In == io)
            error = to_ts_errc(ts_funcs.openCaptureDevice(connection_id, mode_id.data(), device_id.data()));

        return error;
    }

    auto get_current_mode(connection_id_t connection_id, IO io) -> std::tuple<std::error_code, std::string>
    {
        char *result_c = nullptr;
        auto error = ts_errc::parameter_invalid;
        if (IO::Out == io)
            error = to_ts_errc(ts_funcs.getCurrentPlayBackMode(connection_id, &result_c));
        else if (IO::In == io)
            error = to_ts_errc(ts_funcs.getCurrentCaptureMode(connection_id, &result_c));

        auto result = std::string{result_c};
        ts_funcs.freeMemory(result_c);
        return {error, result};
    }

    auto get_current_device_name(connection_id_t connection_id, IO io)
    -> std::tuple<std::error_code, std::string, bool>
    {
        char *result_c = nullptr;
        auto error = ts_errc::parameter_invalid;
        int32_t is_default = 0;
        if (IO::Out == io)
            error = to_ts_errc(ts_funcs.getCurrentPlaybackDeviceName(connection_id, &result_c, &is_default));
        else if (IO::In == io)
            error = to_ts_errc(ts_funcs.getCurrentCaptureDeviceName(connection_id, &result_c, &is_default));

        auto result = std::string{result_c};
        ts_funcs.freeMemory(result_c);
        return {error, result, !!is_default};
    }

    auto close_device(connection_id_t connection_id, IO io, bool graceful) -> std::error_code
    {
        auto error = ts_errc::parameter_invalid;
        if (IO::Out == io)
        {
            if (graceful)
                error = to_ts_errc(ts_funcs.initiateGracefulPlaybackShutdown(connection_id));
            else
                error = to_ts_errc(ts_funcs.closePlaybackDevice(connection_id));
        }
        else if (IO::In == io)
            error = to_ts_errc(ts_funcs.closeCaptureDevice(connection_id));

        return error;
    }

    auto activate_capture_device(connection_id_t connection_id) -> std::error_code
    {
        return to_ts_errc(ts_funcs.activateCaptureDevice(connection_id));
    }

    auto play_wave_file_handle(connection_id_t connection_id, std::string_view path, bool loop)
    -> std::tuple<std::error_code, uint64_t>
    {
        uint64_t handle = 0;
        const auto error =
        to_ts_errc(ts_funcs.playWaveFileHandle(connection_id, path.data(), loop ? 1 : 0, &handle));
        return {error, handle};
    }

    auto pause_wave_file_handle(connection_id_t connection_id, uint64_t wave_handle, bool pause)
    -> std::error_code
    {
        return to_ts_errc(ts_funcs.pauseWaveFileHandle(connection_id, wave_handle, pause ? 1 : 0));
    }

    auto close_wave_file_handle(connection_id_t connection_id, uint64_t wave_handle) -> std::error_code
    {
        return to_ts_errc(ts_funcs.closeWaveFileHandle(connection_id, wave_handle));
    }

    auto play_wave_file(connection_id_t connection_id, std::string_view path) -> std::error_code
    {
        return to_ts_errc(ts_funcs.playWaveFile(connection_id, path.data()));
    }

    auto register_custom_device(std::string_view device_id,
                                std::string_view device_name,
                                int32_t capture_sample_rate,
                                int32_t capture_channels,
                                int32_t playback_sample_rate,
                                int32_t playback_channels) -> std::error_code
    {
        return to_ts_errc(ts_funcs.registerCustomDevice(device_id.data(), device_name.data(),
                                                        capture_sample_rate, capture_channels,
                                                        playback_sample_rate, playback_channels));
    }

    auto unregister_custom_device(std::string_view device_id) -> std::error_code
    {
        return to_ts_errc(ts_funcs.unregisterCustomDevice(device_id.data()));
    }

    // TODO device NAME? really? Also: samples if frame_count(?), cannot do span yet due to that
    auto process_custom_capture_data(std::string_view device_name, const short *buffer, int32_t samples)
    -> std::error_code
    {
        return to_ts_errc(ts_funcs.processCustomCaptureData(device_name.data(), buffer, samples));
    }

    // TODO device NAME? really? Also: samples if frame_count(?), cannot do span yet due to that
    auto acquire_custom_playback_data(std::string_view device_name, short *buffer, int32_t samples)
    -> std::error_code
    {
        return to_ts_errc(ts_funcs.acquireCustomPlaybackData(device_name.data(), buffer, samples));
    }

    /* Preprocessor */
    auto get_preprocessor_info_value_float(connection_id_t connection_id, std::string_view ident)
    -> std::tuple<std::error_code, float>
    {
        float result = 0.f;
        const auto error =
        to_ts_errc(ts_funcs.getPreProcessorInfoValueFloat(connection_id, ident.data(), &result));
        return {error, result};
    }

    auto get_preprocessor_config_value(connection_id_t connection_id, std::string_view ident)
    -> std::tuple<std::error_code, std::string>
    {
        char *result_c = nullptr;
        const auto error =
        to_ts_errc(ts_funcs.getPreProcessorConfigValue(connection_id, ident.data(), &result_c));
        auto result = std::string();
        if (ts_errc::ok == error)
        {
            result = result_c;
            ts_funcs.freeMemory(result_c);
        }
        return {error, result};
    }

    auto set_preprocessor_config_value(connection_id_t connection_id,
                                       std::string_view ident,
                                       std::string_view value) -> std::error_code
    {
        return to_ts_errc(ts_funcs.setPreProcessorConfigValue(connection_id, ident.data(), value.data()));
    }

    /* Encoder */
    auto get_encode_config_value(connection_id_t connection_id, std::string_view ident)
    -> std::tuple<std::error_code, std::string>
    {
        char *result_c = nullptr;
        const auto error = to_ts_errc(ts_funcs.getEncodeConfigValue(connection_id, ident.data(), &result_c));
        auto result = std::string();
        if (ts_errc::ok == error)
        {
            result = result_c;
            ts_funcs.freeMemory(result_c);
        }
        return {error, result};
    }

    /* Playback */
    auto get_playback_config_value_as_float(connection_id_t connection_id, std::string_view ident)
    -> std::tuple<std::error_code, float>
    {
        float result = 0.f;
        const auto error =
        to_ts_errc(ts_funcs.getPlaybackConfigValueAsFloat(connection_id, ident.data(), &result));
        return {error, result};
    }

    auto set_playback_config_value(connection_id_t connection_id,
                                   std::string_view ident,
                                   std::string_view value) -> std::error_code
    {
        return to_ts_errc(ts_funcs.setPlaybackConfigValue(connection_id, ident.data(), value.data()));
    }

    auto set_client_volume_modifier(connection_id_t connection_id, client_id_t client_id, float value)
    -> std::error_code
    {
        return to_ts_errc(ts_funcs.setClientVolumeModifier(connection_id, client_id, value));
    }

    /* Recording */
    auto set_voice_recording(connection_id_t connection_id, bool on_off) -> std::error_code
    {
        if (on_off)
            return to_ts_errc(ts_funcs.startVoiceRecording(connection_id));

        return to_ts_errc(ts_funcs.stopVoiceRecording(connection_id));
    }

    /* 3D sound positioning */
    auto systemset_3d_listener_attributes(connection_id_t connection_id,
                                          const TS3_VECTOR &position,
                                          const TS3_VECTOR &forward,
                                          const TS3_VECTOR &up) -> std::error_code
    {
        return to_ts_errc(ts_funcs.systemset3DListenerAttributes(connection_id, &position, &forward, &up));
    }

    auto set_3d_wave_attributes(connection_id_t connection_id,
                                uint64_t wave_handle,
                                const TS3_VECTOR &position) -> std::error_code
    {
        return to_ts_errc(ts_funcs.set3DWaveAttributes(connection_id, wave_handle, &position));
    }

    auto systemset_3d_settings(connection_id_t connection_id, float distance_factor, float rolloff_scale)
    -> std::error_code
    {
        return to_ts_errc(ts_funcs.systemset3DSettings(connection_id, distance_factor, rolloff_scale));
    }

    auto channelset_3d_attributes(connection_id_t connection_id,
                                  client_id_t client_id,
                                  const TS3_VECTOR &position) -> std::error_code
    {
        return to_ts_errc(ts_funcs.channelset3DAttributes(connection_id, client_id, &position));
    }

}  // namespace sound

/* Interaction with the server */
auto start_connection(connection_id_t connection_id,
                      std::string_view identity,
                      std::string_view ip,
                      uint32_t port,
                      std::string_view nickname,
                      gsl::span<std::string> default_channel_array,
                      std::string_view default_channel_password,
                      std::string_view server_pw) -> std::error_code
{
    if (ip.empty())
        return ts_errc::parameter_invalid;

    if (nickname.empty())
        return ts_errc::parameter_invalid;

    std::vector<const char *> default_channel_array_c;
    for (const auto &channel : default_channel_array)
        default_channel_array_c.push_back(channel.data());

    if (!default_channel_array_c.empty())
        default_channel_array_c.push_back(nullptr);

    return to_ts_errc(ts_funcs.startConnection(
    connection_id, identity.empty() ? nullptr : identity.data(), ip.data(), port, nickname.data(),
    default_channel_array_c.data(), default_channel_password.data(), server_pw.data()));
}

auto stop_connection(connection_id_t connection_id, std::string_view message) -> std::error_code
{
    return to_ts_errc(ts_funcs.stopConnection(connection_id, message.empty() ? nullptr : message.data()));
}

auto request_client_move(connection_id_t connection_id,
                         client_id_t client_id,
                         channel_id_t new_channel_id,
                         std::string_view password /*= ""*/,
                         std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestClientMove(connection_id, client_id, new_channel_id,
                                                 password.empty() ? nullptr : password.data(),
                                                 return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_properties(connection_id_t connection_id,
                               client_id_t client_id,
                               std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestClientVariables(connection_id, client_id,
                                                      return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_kick(connection_id_t connection_id,
                         client_id_t client_id,
                         Kick_From kick_from,
                         std::string_view reason,
                         std::string_view return_code /*= ""*/) -> std::error_code
{
    if (Kick_From::Server == kick_from)
        return to_ts_errc(ts_funcs.requestClientKickFromServer(
        connection_id, client_id, reason.data(), return_code.empty() ? nullptr : return_code.data()));
    else if (Kick_From::Channel == kick_from)
        return to_ts_errc(ts_funcs.requestClientKickFromChannel(
        connection_id, client_id, reason.data(), return_code.empty() ? nullptr : return_code.data()));

    return ts_errc::parameter_invalid;
}

auto request_channel_delete(connection_id_t connection_id,
                            channel_id_t channel_id,
                            bool force,
                            std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestChannelDelete(connection_id, channel_id, force ? 1 : 0,
                                                    return_code.empty() ? nullptr : return_code.data()));
}

auto request_channel_move(connection_id_t connection_id,
                          channel_id_t channel_id,
                          channel_id_t new_channel_parent_id,
                          channel_id_t new_channel_order,
                          std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestChannelMove(connection_id, channel_id, new_channel_parent_id,
                                                  new_channel_order,
                                                  return_code.empty() ? nullptr : return_code.data()));
}

auto request_send_text_msg_private(connection_id_t connection_id,
                                   std::string_view message,
                                   client_id_t target_client_id,
                                   std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestSendPrivateTextMsg(connection_id, message.data(), target_client_id,
                                                         return_code.empty() ? nullptr : return_code.data()));
}

auto request_send_text_msg_channel(connection_id_t connection_id,
                                   std::string_view message,
                                   channel_id_t target_channel_id,
                                   std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestSendChannelTextMsg(connection_id, message.data(), target_channel_id,
                                                         return_code.empty() ? nullptr : return_code.data()));
}

auto request_send_text_msg_server(connection_id_t connection_id,
                                  std::string_view message,
                                  std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestSendServerTextMsg(connection_id, message.data(),
                                                        return_code.empty() ? nullptr : return_code.data()));
}

auto request_connection_info(connection_id_t connection_id,
                             client_id_t client_id,
                             std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestConnectionInfo(connection_id, client_id,
                                                     return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_set_whisperlist(connection_id_t connection_id,
                                    client_id_t client_id,
                                    gsl::span<channel_id_t> target_channel_ids,
                                    gsl::span<client_id_t> target_client_ids,
                                    std::string_view return_code /*= ""*/) -> std::error_code
{
    auto channel_ids = std::vector<channel_id_t>(target_channel_ids.begin(), target_channel_ids.end());
    if (!channel_ids.empty())
    {
        if (0 != channel_ids.back())
            channel_ids.push_back(0);
    }
    auto client_ids = std::vector<client_id_t>(target_client_ids.begin(), target_client_ids.end());
    if (!client_ids.empty())
    {
        if (0 != client_ids.back())
            client_ids.push_back(0);
    }
    return to_ts_errc(
    ts_funcs.requestClientSetWhisperList(connection_id, client_id, channel_ids.data(), client_ids.data(),
                                         return_code.empty() ? nullptr : return_code.data()));
}

auto request_channel_subscribe(connection_id_t connection_id,
                               bool onoff,
                               gsl::span<channel_id_t> channel_ids,
                               std::string_view return_code /*= ""*/) -> std::error_code
{
    if (onoff)
        return to_ts_errc(ts_funcs.requestChannelSubscribe(
        connection_id, channel_ids.data(), return_code.empty() ? nullptr : return_code.data()));

    return to_ts_errc(ts_funcs.requestChannelUnsubscribe(connection_id, channel_ids.data(),
                                                         return_code.empty() ? nullptr : return_code.data()));
}

auto request_channel_subscribe_all(connection_id_t connection_id,
                                   bool onoff,
                                   std::string_view return_code /*= ""*/) -> std::error_code
{
    if (onoff)
        return to_ts_errc(ts_funcs.requestChannelSubscribeAll(
        connection_id, return_code.empty() ? nullptr : return_code.data()));

    return to_ts_errc(
    ts_funcs.requestChannelUnsubscribeAll(connection_id, return_code.empty() ? nullptr : return_code.data()));
}

auto request_channel_description(connection_id_t connection_id,
                                 channel_id_t channel_id,
                                 std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestChannelDescription(connection_id, channel_id,
                                                         return_code.empty() ? nullptr : return_code.data()));
}

auto request_mute_clients(connection_id_t connection_id,
                          gsl::span<client_id_t> client_ids,
                          bool mute,
                          bool temporary,
                          std::string_view return_code /* = ""*/) -> std::error_code
{
    auto error = ts_errc::parameter_invalid;
    if (client_ids.empty())
        return error;

    auto client_ids_ = std::vector<client_id_t>(client_ids.begin(), client_ids.end());
    if (client_ids_.back() != 0)
        client_ids_.push_back(0);

    if (mute)
    {
        if (temporary)
        {
            return to_ts_errc(ts_funcs.requestMuteClientsTemporary(
            connection_id, client_ids_.data(), return_code.empty() ? nullptr : return_code.data()));
        }
        return to_ts_errc(ts_funcs.requestMuteClients(connection_id, client_ids_.data(),
                                                      return_code.empty() ? nullptr : return_code.data()));
    }

    if (temporary)
    {
        return to_ts_errc(ts_funcs.requestUnmuteClientsTemporary(
        connection_id, client_ids.data(), return_code.empty() ? nullptr : return_code.data()));
    }
    return to_ts_errc(ts_funcs.requestUnmuteClients(connection_id, client_ids.data(),
                                                    return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_poke(connection_id_t connection_id,
                         client_id_t client_id,
                         std::string_view message,
                         std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestClientPoke(connection_id, client_id, message.data(),
                                                 return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_ids(connection_id_t connection_id,
                        std::string_view client_uid,
                        std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestClientIDs(connection_id, client_uid.data(),
                                                return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_chat_closed(connection_id_t connection_id,
                                std::string_view client_uid,
                                client_id_t client_id,
                                std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.clientChatClosed(connection_id, client_uid.data(), client_id,
                                                return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_chat_composing(connection_id_t connection_id,
                                   client_id_t client_id,
                                   std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.clientChatComposing(connection_id, client_id,
                                                   return_code.empty() ? nullptr : return_code.data()));
}

namespace server_temporary_password
{
    auto request_server_temporary_password_add(connection_id_t connection_id,
                                               std::string_view password,
                                               std::string_view description,
                                               uint64_t duration,
                                               channel_id_t target_channel_id,
                                               std::string_view target_channel_pw,
                                               std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestServerTemporaryPasswordAdd(
        connection_id, password.data(), description.data(), duration, target_channel_id,
        target_channel_pw.data(), return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_del(connection_id_t connection_id,
                     std::string_view pw,
                     std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestServerTemporaryPasswordDel(
        connection_id, pw.data(), return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_list(connection_id_t connection_id, std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestServerTemporaryPasswordList(
        connection_id, return_code.empty() ? nullptr : return_code.data()));
    }

}  // namespace server_temporary_password

/* Access clientlib information */

/* Query own client ID */
auto get_client_id(connection_id_t connection_id) -> std::tuple<std::error_code, client_id_t>
{
    client_id_t my_id = 0;
    const auto error = to_ts_errc(ts_funcs.getClientID(connection_id, &my_id));
    return {error, my_id};
}

/* Client info */

auto get_client_self_property_as_int(connection_id_t connection_id, size_t flag)
-> std::tuple<std::error_code, int32_t>
{
    auto value = int32_t{0};
    const auto error = to_ts_errc(ts_funcs.getClientSelfVariableAsInt(connection_id, flag, &value));
    return {error, (error == ts_errc::ok) ? value : 0};
}

auto get_client_self_property_as_string(connection_id_t connection_id, size_t flag)
-> std::tuple<std::error_code, std::string>
{
    std::string result;
    char *result_c = nullptr;
    const auto error = to_ts_errc(ts_funcs.getClientSelfVariableAsString(connection_id, flag, &result_c));
    if (ts_errc::ok == error)
    {
        result = std::string(result_c);
        ts_funcs.freeMemory(result_c);
    }
    return {error, result};
}

auto set_client_self_property_as_int(connection_id_t connection_id, size_t flag, int32_t value)
-> std::error_code
{
    return to_ts_errc(ts_funcs.setClientSelfVariableAsInt(connection_id, flag, value));
}

auto set_client_self_property_as_string(connection_id_t connection_id, size_t flag, std::string_view value)
-> std::error_code
{
    return to_ts_errc(ts_funcs.setClientSelfVariableAsString(connection_id, flag, value.data()));
}

auto flush_client_self_updates(connection_id_t connection_id, std::string_view return_code) -> std::error_code
{
    return to_ts_errc(
    ts_funcs.flushClientSelfUpdates(connection_id, return_code.empty() ? nullptr : return_code.data()));
}

auto get_client_property_as_int(connection_id_t connection_id, client_id_t client_id, size_t flag)
-> std::tuple<std::error_code, int32_t>
{
    auto value = int32_t{0};
    const auto error = to_ts_errc(ts_funcs.getClientVariableAsInt(connection_id, client_id, flag, &value));
    return {error, (ts_errc::ok == error) ? value : 0};
}

auto get_client_property_as_uint64(connection_id_t connection_id, client_id_t client_id, size_t flag)
-> std::tuple<std::error_code, uint64_t>
{
    auto value = uint64_t{0};
    const auto error = to_ts_errc(ts_funcs.getClientVariableAsUInt64(connection_id, client_id, flag, &value));
    return {error, (ts_errc::ok == error) ? value : 0};
}

auto get_client_property_as_string(connection_id_t connection_id, client_id_t client_id, size_t flag)
-> std::tuple<std::error_code, std::string>
{
    std::string result;
    char *result_c = nullptr;
    const auto error =
    to_ts_errc(ts_funcs.getClientVariableAsString(connection_id, client_id, flag, &result_c));
    if (ts_errc::ok == error)
    {
        result = std::string(result_c);
        ts_funcs.freeMemory(result_c);
    }
    return {error, result};
}

auto get_client_ids(connection_id_t connection_id)
-> std::tuple<std::error_code, std::vector<com::teamspeak::client_id_t>>
{
    std::vector<client_id_t> result;
    client_id_t *client_ids_c = nullptr;
    const auto error = to_ts_errc(ts_funcs.getClientList(connection_id, &client_ids_c));
    if (ts_errc::ok == error)
    {
        for (int32_t i = 0; client_ids_c[i]; ++i)
            result.push_back(client_ids_c[i]);

        ts_funcs.freeMemory(client_ids_c);
    }
    return {error, result};
}

auto get_channel_of_client(connection_id_t connection_id, client_id_t client_id)
-> std::tuple<std::error_code, channel_id_t>
{
    uint64_t value = 0;
    const auto error = to_ts_errc(ts_funcs.getChannelOfClient(connection_id, client_id, &value));
    return {error, (ts_errc::ok == error) ? value : 0};
}

/* Channel info */

auto get_channel_property_as_int(connection_id_t connection_id, channel_id_t channel_id, size_t flag)
-> std::tuple<std::error_code, int32_t>
{
    int32_t value = 0;
    const auto error = to_ts_errc(ts_funcs.getChannelVariableAsInt(connection_id, channel_id, flag, &value));
    return {error, value};
}

auto get_channel_property_as_uint64(connection_id_t connection_id, channel_id_t channel_id, size_t flag)
-> std::tuple<std::error_code, uint64_t>
{
    uint64_t value = 0;
    const auto error =
    to_ts_errc(ts_funcs.getChannelVariableAsUInt64(connection_id, channel_id, flag, &value));
    return {error, value};
}

auto get_channel_property_as_string(connection_id_t connection_id, channel_id_t channel_id, size_t flag)
-> std::tuple<std::error_code, std::string>
{
    std::string result;
    char *result_c = nullptr;
    const auto error =
    to_ts_errc(ts_funcs.getChannelVariableAsString(connection_id, channel_id, flag, &result_c));
    if (ts_errc::ok == error)
    {
        result = std::string(result_c);
        ts_funcs.freeMemory(result_c);
    }
    return {error, result};
}

auto get_channel_id_from_channel_names(connection_id_t connection_id, gsl::span<std::string> channel_names)
-> std::tuple<std::error_code, channel_id_t>
{
    channel_id_t result = 0;
    std::vector<char *> channel_names_c;
    channel_names_c.reserve(channel_names.size() + 1u);

    std::vector<std::vector<char>> channel_names_;
    channel_names_.reserve(channel_names.size());
    for (auto &&channel_name : channel_names)
    {
        // convert channel name string to char vector and emplace it in our life time container...
        // a const cast probably would've done it, but meh
        channel_names_.emplace_back(channel_name.data(), channel_name.data() + channel_name.size() + 1u);
        // push back a pointer to that entry to the vector we give to C...
        channel_names_c.push_back(channel_names_.back().data());
    }
    channel_names_c.push_back(nullptr);
    const auto error =
    to_ts_errc(ts_funcs.getChannelIDFromChannelNames(connection_id, channel_names_c.data(), &result));
    return {error, result};
}

auto set_channel_variable_as_int(connection_id_t connection_id,
                                 channel_id_t channel_id,
                                 size_t flag,
                                 int32_t value) -> std::error_code
{
    return to_ts_errc(ts_funcs.setChannelVariableAsInt(connection_id, channel_id, flag, value));
}

auto set_channel_variable_as_uint64(connection_id_t connection_id,
                                    channel_id_t channel_id,
                                    size_t flag,
                                    uint64_t value) -> std::error_code
{
    return to_ts_errc(ts_funcs.setChannelVariableAsUInt64(connection_id, channel_id, flag, value));
}

auto set_channel_variable_as_string(connection_id_t connection_id,
                                    channel_id_t channel_id,
                                    size_t flag,
                                    std::string_view value) -> std::error_code
{
    return to_ts_errc(ts_funcs.setChannelVariableAsString(connection_id, channel_id, flag, value.data()));
}

auto flush_channel_updates(connection_id_t connection_id,
                           channel_id_t channel_id,
                           std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.flushChannelUpdates(connection_id, channel_id,
                                                   return_code.empty() ? nullptr : return_code.data()));
}

auto flush_channel_creation(connection_id_t connection_id,
                            channel_id_t channel_parent_id,
                            std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.flushChannelCreation(connection_id, channel_parent_id,
                                                    return_code.empty() ? nullptr : return_code.data()));
}

auto get_channel_ids(connection_id_t connection_id) -> std::tuple<std::error_code, std::vector<channel_id_t>>
{
    std::vector<channel_id_t> result;
    channel_id_t *channel_ids_c = nullptr;
    const auto error = to_ts_errc(ts_funcs.getChannelList(connection_id, &channel_ids_c));
    if (ts_errc::ok == error)
    {
        for (int32_t i = 0; channel_ids_c[i]; ++i)
            result.push_back(channel_ids_c[i]);

        ts_funcs.freeMemory(channel_ids_c);
    }
    return {error, result};
}

auto get_channel_id(connection_id_t connection_id) -> std::tuple<std::error_code, channel_id_t>
{
    const auto [error_client_id, client_id] = get_client_id(connection_id);
    if (ts_errc::ok != error_client_id)
        return {error_client_id, client_id};

    const auto [error_channel_id, channel_id] = get_channel_of_client(connection_id, client_id);
    return {error_channel_id, channel_id};
}

auto get_channel_client_ids(connection_id_t connection_id, channel_id_t channel_id)
-> std::tuple<std::error_code, std::vector<client_id_t>>
{
    std::vector<client_id_t> result;
    client_id_t *client_ids_c = nullptr;
    const auto error = to_ts_errc(ts_funcs.getChannelClientList(connection_id, channel_id, &client_ids_c));
    if (ts_errc::ok == error)
    {
        for (int32_t i = 0; client_ids_c[i]; ++i)
            result.push_back(client_ids_c[i]);

        ts_funcs.freeMemory(client_ids_c);
    }
    return {error, result};
}

auto get_parent_channel_of_channel(connection_id_t connection_id, channel_id_t channelID)
-> std::tuple<std::error_code, channel_id_t>
{
    channel_id_t result = 0;
    const auto error = to_ts_errc(ts_funcs.getParentChannelOfChannel(connection_id, channelID, &result));
    return {error, result};
}

/* Server info */

auto get_server_connection_handler_ids() -> std::tuple<std::error_code, std::vector<connection_id_t>>
{
    std::vector<connection_id_t> result;
    connection_id_t *connection_ids_c = nullptr;
    const auto error = to_ts_errc(ts_funcs.getServerConnectionHandlerList(&connection_ids_c));
    if (ts_errc::ok == error)
    {
        for (int32_t i = 0; connection_ids_c[i]; ++i)
            result.push_back(connection_ids_c[i]);

        ts_funcs.freeMemory(connection_ids_c);
    }
    return {error, result};
}

auto get_server_property_as_int(connection_id_t connection_id, size_t flag)
-> std::tuple<std::error_code, int32_t>
{
    int32_t result = 0;
    const auto error = to_ts_errc(ts_funcs.getServerVariableAsInt(connection_id, flag, &result));
    return {error, result};
}

auto get_server_property_as_uint64(connection_id_t connection_id, size_t flag)
-> std::tuple<std::error_code, uint64_t>
{
    uint64_t result = 0;
    const auto error = to_ts_errc(ts_funcs.getServerVariableAsUInt64(connection_id, flag, &result));
    return {error, result};
}

auto get_server_property_as_string(connection_id_t connection_id, size_t flag)
-> std::tuple<std::error_code, std::string>
{
    std::string result;
    char *result_c = nullptr;
    const auto error = to_ts_errc(ts_funcs.getServerVariableAsString(connection_id, flag, &result_c));
    if (ts_errc::ok == error)
    {
        result = std::string(result_c);
        ts_funcs.freeMemory(result_c);
    }
    return {error, result};
}

auto request_server_properties(connection_id_t connection_id) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestServerVariables(connection_id));
}

/* Connection info */
auto get_connection_status(connection_id_t connection_id) -> std::tuple<std::error_code, int>
{
    int result = 0;
    const auto error = to_ts_errc(ts_funcs.getConnectionStatus(connection_id, &result));
    return {error, result};
}

auto get_connection_property_as_uint64(connection_id_t connection_id, client_id_t client_id, size_t flag)
-> std::tuple<std::error_code, uint64_t>
{
    uint64_t result = 0;
    const auto error =
    to_ts_errc(ts_funcs.getConnectionVariableAsUInt64(connection_id, client_id, flag, &result));
    return {error, result};
}

auto get_connection_property_as_double(connection_id_t connection_id, client_id_t client_id, size_t flag)
-> std::tuple<std::error_code, double>
{
    double result = 0;
    const auto error =
    to_ts_errc(ts_funcs.getConnectionVariableAsDouble(connection_id, client_id, flag, &result));
    return {error, result};
}

auto get_connection_property_as_string(connection_id_t connection_id, client_id_t client_id, size_t flag)
-> std::tuple<std::error_code, std::string>
{
    char *result_c = nullptr;
    const auto error =
    to_ts_errc(ts_funcs.getConnectionVariableAsString(connection_id, client_id, flag, &result_c));
    const auto result = ts_errc::ok != error ? std::string{} : std::string{result_c};
    if (ts_errc::ok == error)
        ts_funcs.freeMemory(result_c);

    return {error, result};
}

auto clean_up_connection_info(connection_id_t connection_id, client_id_t client_id) -> std::error_code
{
    return to_ts_errc(ts_funcs.cleanUpConnectionInfo(connection_id, client_id));
}

/* Client related */
auto request_client_db_id_from_uid(connection_id_t connection_id,
                                   std::string_view client_uid,
                                   std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestClientDBIDfromUID(connection_id, client_uid.data(),
                                                        return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_name_from_uid(connection_id_t connection_id,
                                  std::string_view client_uid,
                                  std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestClientNamefromUID(connection_id, client_uid.data(),
                                                        return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_name_from_dbid(connection_id_t connection_id,
                                   client_db_id_t client_db_id,
                                   std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestClientNamefromDBID(connection_id, client_db_id,
                                                         return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_edit_description(connection_id_t connection_id,
                                     client_id_t client_id,
                                     std::string_view client_description,
                                     std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestClientEditDescription(
    connection_id, client_id, client_description.data(), return_code.empty() ? nullptr : return_code.data()));
}

auto request_client_set_is_talker(connection_id_t connection_id,
                                  client_id_t client_id,
                                  bool is_talker,
                                  std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestClientSetIsTalker(connection_id, client_id, is_talker ? 1 : 0,
                                                        return_code.empty() ? nullptr : return_code.data()));
}

auto request_is_talker(connection_id_t connection_id,
                       bool is_talker_request,
                       std::string_view message,
                       std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestIsTalker(connection_id, is_talker_request ? 1 : 0, message.data(),
                                               return_code.empty() ? nullptr : return_code.data()));
}

/* Plugin related */
auto request_send_client_query_command(connection_id_t connection_id,
                                       std::string_view command,
                                       std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.requestSendClientQueryCommand(
    connection_id, command.data(), return_code.empty() ? nullptr : return_code.data()));
}

/* Filetransfer */
namespace file_transfer
{

    auto get_transfer_file_name(transfer_id_t transfer_id) -> std::tuple<std::error_code, std::string>
    {
        char *result_c = nullptr;
        const auto error = to_ts_errc(ts_funcs.getTransferFileName(transfer_id, &result_c));
        std::string result = ts_errc::ok != error ? std::string{} : std::string{result_c};
        if (ts_errc::ok == error)
            ts_funcs.freeMemory(result_c);

        return {error, result};
    }

    auto get_transfer_file_path(transfer_id_t transfer_id) -> std::tuple<std::error_code, std::string>
    {
        char *result_c = nullptr;
        const auto error = to_ts_errc(ts_funcs.getTransferFilePath(transfer_id, &result_c));
        std::string result = ts_errc::ok != error ? std::string{} : std::string{result_c};
        if (ts_errc::ok == error)
            ts_funcs.freeMemory(result_c);

        return {error, result};
    }

    auto get_transfer_file_size(transfer_id_t transfer_id) -> std::tuple<std::error_code, uint64_t>
    {
        auto result = uint64_t{0};
        const auto error = to_ts_errc(ts_funcs.getTransferFileSize(transfer_id, &result));
        return {error, result};
    }

    auto get_transfer_file_size_done(transfer_id_t transfer_id) -> std::tuple<std::error_code, uint64_t>
    {
        auto result = uint64_t{0};
        const auto error = to_ts_errc(ts_funcs.getTransferFileSizeDone(transfer_id, &result));
        return {error, result};
    }

    auto is_transfer_sender(transfer_id_t transfer_id) -> std::tuple<std::error_code, bool>
    {
        auto result = int32_t{0};
        const auto error = to_ts_errc(ts_funcs.isTransferSender(transfer_id, &result));
        return {error, !!result};
    }

    auto get_transfer_status(transfer_id_t transfer_id) -> std::tuple<std::error_code, int32_t>
    {
        auto result = int32_t{0};
        const auto error = to_ts_errc(ts_funcs.getTransferStatus(transfer_id, &result));
        return {error, result};
    }

    auto get_transfer_speed_current(transfer_id_t transfer_id) -> std::tuple<std::error_code, float>
    {
        auto result = float{0.F};
        const auto error = to_ts_errc(ts_funcs.getCurrentTransferSpeed(transfer_id, &result));
        return {error, result};
    }

    auto get_transfer_speed_average(transfer_id_t transfer_id) -> std::tuple<std::error_code, float>
    {
        auto result = float{0.F};
        const auto error = to_ts_errc(ts_funcs.getAverageTransferSpeed(transfer_id, &result));
        return {error, result};
    }

    auto get_transfer_run_time(transfer_id_t transfer_id) -> std::tuple<std::error_code, uint64_t>
    {
        auto result = uint64_t{0};
        const auto error = to_ts_errc(ts_funcs.getTransferRunTime(transfer_id, &result));
        return {error, result};
    }

    auto send_file(connection_id_t connection_id,
                   channel_id_t channel_id,
                   std::string_view channel_pw,
                   std::string_view file,
                   bool overwrite,
                   bool resume,
                   std::string_view source_directory,
                   std::string_view return_code /*= ""*/) -> std::tuple<std::error_code, transfer_id_t>
    {
        auto result = transfer_id_t{0};
        const auto error = to_ts_errc(ts_funcs.sendFile(
        connection_id, channel_id, channel_pw.data(), file.data(), overwrite ? 1 : 0, resume ? 1 : 0,
        source_directory.data(), &result, return_code.empty() ? nullptr : return_code.data()));
        return {error, result};
    }

    auto request_file(connection_id_t connection_id,
                      channel_id_t channel_id,
                      std::string_view channel_pw,
                      std::string_view file,
                      bool overwrite,
                      bool resume,
                      std::string_view destination_directory,
                      std::string_view return_code /*= ""*/) -> std::tuple<std::error_code, transfer_id_t>
    {
        auto result = transfer_id_t{0};
        const auto error = to_ts_errc(ts_funcs.requestFile(
        connection_id, channel_id, channel_pw.data(), file.data(), overwrite ? 1 : 0, resume ? 1 : 0,
        destination_directory.data(), &result, return_code.empty() ? nullptr : return_code.data()));
        return {error, result};
    }

    auto halt_Transfer(connection_id_t connection_id,
                       transfer_id_t transfer_id,
                       bool delete_unfinished_file,
                       std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.haltTransfer(connection_id, transfer_id, delete_unfinished_file ? 1 : 0,
                                                return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_file_list(connection_id_t connection_id,
                           channel_id_t channel_id,
                           std::string_view channel_pw,
                           std::string_view path,
                           std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestFileList(connection_id, channel_id, channel_pw.data(), path.data(),
                                                   return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_file_info(connection_id_t connection_id,
                           channel_id_t channel_id,
                           std::string_view channel_pw,
                           std::string_view file,
                           std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestFileInfo(connection_id, channel_id, channel_pw.data(), file.data(),
                                                   return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_delete_files(connection_id_t connection_id,
                              channel_id_t channel_id,
                              std::string_view channel_pw,
                              gsl::span<std::string> full_file_paths,
                              std::string_view return_code /*= ""*/) -> std::error_code
    {
        if (full_file_paths.empty())
            return ts_errc::parameter_invalid;

        auto paths = std::vector<const char *>{};
        for (const auto &path : full_file_paths)
            paths.push_back(path.data());

        if (paths.back() != '\0')
            paths.push_back(nullptr);

        // TODO: yeah, it's actually plural
        return to_ts_errc(ts_funcs.requestDeleteFile(
        connection_id, channel_id, channel_pw.empty() ? nullptr : channel_pw.data(), paths.data(),
        return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_create_directory(connection_id_t connection_id,
                                  channel_id_t channel_id,
                                  std::string_view channel_pw,
                                  std::string_view directory_path,
                                  std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(
        ts_funcs.requestCreateDirectory(connection_id, channel_id, channel_pw.data(), directory_path.data(),
                                        return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_rename_file(connection_id_t connection_id,
                             channel_id_t from_channel_id,
                             std::string_view channel_pw,
                             channel_id_t to_channel_id,
                             std::string_view to_channel_pw,
                             std::string_view old_file,
                             std::string_view new_file,
                             std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestRenameFile(
        connection_id, from_channel_id, channel_pw.data(), to_channel_id, to_channel_pw.data(),
        old_file.data(), new_file.data(), return_code.empty() ? nullptr : return_code.data()));
    }

}  // namespace file_transfer

/* Offline message management */
namespace offline_messages
{


    auto request_add(connection_id_t connection_id,
                     std::string_view to_client_uid,
                     std::string_view subject,
                     std::string_view message,
                     std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestMessageAdd(connection_id, to_client_uid.data(), subject.data(),
                                                     message.data(),
                                                     return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_del(connection_id_t connection_id,
                     uint64_t message_id,
                     std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestMessageDel(connection_id, message_id,
                                                     return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_get(connection_id_t connection_id,
                     uint64_t message_id,
                     std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestMessageGet(connection_id, message_id,
                                                     return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_list(connection_id_t connection_id, std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(
        ts_funcs.requestMessageList(connection_id, return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_update_flag(connection_id_t connection_id,
                             uint64_t message_id,
                             int32_t flag,
                             std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestMessageUpdateFlag(
        connection_id, message_id, flag, return_code.empty() ? nullptr : return_code.data()));
    }

}  // namespace offline_messages

/* Interacting with the server - confirming passwords */

auto verify_password_server(connection_id_t connection_id,
                            std::string_view server_pw,
                            std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.verifyServerPassword(connection_id, server_pw.data(),
                                                    return_code.empty() ? nullptr : return_code.data()));
}

auto verify_password_channel(connection_id_t connection_id,
                             channel_id_t channel_id,
                             std::string_view channel_pw,
                             std::string_view return_code /*= ""*/) -> std::error_code
{
    return to_ts_errc(ts_funcs.verifyChannelPassword(connection_id, channel_id, channel_pw.data(),
                                                     return_code.empty() ? nullptr : return_code.data()));
}

/* Interacting with the server - banning */
namespace ban
{

    auto ban_client(connection_id_t connection_id,
                    client_id_t client_id,
                    uint64_t time_in_seconds,
                    std::string_view reason,
                    std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.banclient(connection_id, client_id, time_in_seconds, reason.data(),
                                             return_code.empty() ? nullptr : return_code.data()));
    }

    auto ban_add(connection_id_t connection_id,
                 std::string_view ip_regexp,
                 std::string_view name_regexp,
                 std::string_view target_client_uid,
                 std::string_view target_client_mytsid,
                 uint64_t time_in_seconds,
                 std::string_view reason,
                 std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.banadd(connection_id, ip_regexp.data(), name_regexp.data(),
                                          target_client_uid.data(), target_client_mytsid.data(),
                                          time_in_seconds, reason.data(),
                                          return_code.empty() ? nullptr : return_code.data()));
    }

    auto ban_client_db_id(connection_id_t connection_id,
                          client_db_id_t client_db_id,
                          uint64_t time_in_seconds,
                          std::string_view reason,
                          std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.banclientdbid(connection_id, client_db_id, time_in_seconds, reason.data(),
                                                 return_code.empty() ? nullptr : return_code.data()));
    }

    auto ban_del(connection_id_t connection_id, uint64_t ban_id, std::string_view return_code /*= ""*/)
    -> std::error_code
    {
        return to_ts_errc(
        ts_funcs.bandel(connection_id, ban_id, return_code.empty() ? nullptr : return_code.data()));
    }

    auto ban_del_all(connection_id_t connection_id, std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(
        ts_funcs.bandelall(connection_id, return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_list(connection_id_t connection_id,
                      uint64_t start,
                      uint32_t duration,
                      std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestBanList(connection_id, start, duration,
                                                  return_code.empty() ? nullptr : return_code.data()));
    }

}  // namespace ban

/* Interacting with the server - complain */
namespace complain
{
    auto request_add(connection_id_t connection_id,
                     client_db_id_t targetClientDatabaseID,
                     std::string_view reason,
                     std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestComplainAdd(connection_id, targetClientDatabaseID, reason.data(),
                                                      return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_del(connection_id_t connection_id,
                     client_db_id_t targetClientDatabaseID,
                     client_db_id_t fromClientDatabaseID,
                     std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestComplainDel(connection_id, targetClientDatabaseID,
                                                      fromClientDatabaseID,
                                                      return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_del_all(connection_id_t connection_id,
                         client_db_id_t targetClientDatabaseID,
                         std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestComplainDelAll(connection_id, targetClientDatabaseID,
                                                         return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_list(connection_id_t connection_id,
                      client_db_id_t targetClientDatabaseID,
                      std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestComplainList(connection_id, targetClientDatabaseID,
                                                       return_code.empty() ? nullptr : return_code.data()));
    }

}  // namespace complain

/* Permissions */

namespace permissions
{

    auto get_permission_name(connection_id_t connection_id, permission_id_t permission_id)
    -> std::tuple<std::error_code, std::string>
    {
        std::string result;
        auto result_c = std::array<char, kPermBufferSize>{};
        const auto error = to_ts_errc(
        ts_funcs.getPermissionNameByID(connection_id, permission_id, result_c.data(), kPermBufferSize));
        if (ts_errc::ok == error)
            result = std::string(result_c.data());

        return {error, result};
    }

    auto request_group_list(connection_id_t connection_id,
                            Group_List_Type group_list_type,
                            std::string_view return_code) -> std::error_code
    {
        switch (group_list_type)
        {
        case Group_List_Type::Server:
            return to_ts_errc(ts_funcs.requestServerGroupList(
            connection_id, return_code.empty() ? nullptr : return_code.data()));
        case Group_List_Type::Channel:
            return to_ts_errc(ts_funcs.requestChannelGroupList(
            connection_id, return_code.empty() ? nullptr : return_code.data()));
        default:
            break;
        }
        return ts_errc::parameter_invalid;
    }

    auto request_group_add(connection_id_t connection_id,
                           Group_List_Type group_list_type,
                           std::string_view group_name,
                           int32_t group_type,
                           std::string_view return_code) -> std::error_code
    {
        switch (group_list_type)
        {
        case Group_List_Type::Server:
            return to_ts_errc(
            ts_funcs.requestServerGroupAdd(connection_id, group_name.data(), group_type,
                                           return_code.empty() ? nullptr : return_code.data()));
        case Group_List_Type::Channel:
            return to_ts_errc(
            ts_funcs.requestChannelGroupAdd(connection_id, group_name.data(), group_type,
                                            return_code.empty() ? nullptr : return_code.data()));
        default:
            break;
        }
        return ts_errc::parameter_invalid;
    }

    auto request_group_del(connection_id_t connection_id,
                           Group_List_Type group_list_type,
                           uint64_t group_id,
                           bool force,
                           std::string_view return_code) -> std::error_code
    {
        switch (group_list_type)
        {
        case Group_List_Type::Server:
            return to_ts_errc(ts_funcs.requestServerGroupDel(
            connection_id, group_id, force ? 1 : 0, return_code.empty() ? nullptr : return_code.data()));
        case Group_List_Type::Channel:
            return to_ts_errc(ts_funcs.requestChannelGroupDel(
            connection_id, group_id, force ? 1 : 0, return_code.empty() ? nullptr : return_code.data()));
        default:
            break;
        }
        return ts_errc::parameter_invalid;
    }

    namespace server_group
    {

        auto request_change_client(connection_id_t connection_id,
                                   bool add_remove,
                                   uint64_t group_id,
                                   client_db_id_t client_db_id,
                                   std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (add_remove)
            {
                return to_ts_errc(ts_funcs.requestServerGroupAddClient(
                connection_id, group_id, client_db_id, return_code.empty() ? nullptr : return_code.data()));
            }
            return to_ts_errc(ts_funcs.requestServerGroupDelClient(
            connection_id, group_id, client_db_id, return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_groups_by_client_id(connection_id_t connection_id,
                                         client_db_id_t clientDBID,
                                         std::string_view return_code /*= ""*/) -> std::error_code
        {
            return to_ts_errc(ts_funcs.requestServerGroupsByClientID(
            connection_id, clientDBID, return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_add_perm(connection_id_t connection_id,
                              permission_group_id_t server_group_id,
                              bool continue_on_error,
                              gsl::span<Permission_Entry> entries,
                              std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (entries.empty())
                return ts_errc::parameter_invalid;

            std::vector<permission_id_t> permission_ids;
            std::vector<int32_t> values;
            std::vector<int32_t> negated;
            std::vector<int32_t> skip;

            for (const auto &entry : entries)
            {
                permission_ids.push_back(entry.id);
                values.push_back(entry.value);
                negated.push_back(entry.negated);
                skip.push_back(entry.skip);
            }

            return to_ts_errc(ts_funcs.requestServerGroupAddPerm(
            connection_id, server_group_id, continue_on_error ? 1 : 0, permission_ids.data(), values.data(),
            negated.data(), skip.data(), permission_ids.size(),
            return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_del_perm(connection_id_t connection_id,
                              permission_group_id_t server_group_id,
                              bool continue_on_error,
                              gsl::span<permission_id_t> ids,
                              std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (ids.empty())
                return ts_errc::parameter_invalid;

            return to_ts_errc(ts_funcs.requestServerGroupDelPerm(
            connection_id, server_group_id, continue_on_error ? 1 : 0, ids.data(), ids.size(),
            return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_list(connection_id_t connection_id,
                          permission_group_id_t server_group_id,
                          std::string_view return_code /*= ""*/) -> std::error_code
        {
            return to_ts_errc(ts_funcs.requestServerGroupPermList(
            connection_id, server_group_id, return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_client_list(connection_id_t connection_id,
                                 permission_group_id_t group_id,
                                 bool with_names,
                                 std::string_view return_code) -> std::error_code
        {
            return to_ts_errc(ts_funcs.requestServerGroupClientList(
            connection_id, group_id, with_names ? 1 : 0, return_code.empty() ? nullptr : return_code.data()));
        }
    }  // namespace server_group

    namespace channel_group
    {
        auto request_add_perm(connection_id_t connection_id,
                              permission_group_id_t group_id,
                              bool continue_on_error,
                              gsl::span<Permission_Entry> entries,
                              std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (entries.empty())
                return ts_errc::parameter_invalid;

            std::vector<permission_id_t> permission_ids;
            std::vector<int32_t> values;

            for (const auto &entry : entries)
            {
                permission_ids.push_back(entry.id);
                values.push_back(entry.value);
            }

            return to_ts_errc(ts_funcs.requestChannelGroupAddPerm(
            connection_id, group_id, continue_on_error ? 1 : 0, permission_ids.data(), values.data(),
            permission_ids.size(), return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_del_perm(connection_id_t connection_id,
                              permission_group_id_t group_id,
                              bool continue_on_error,
                              gsl::span<permission_id_t> ids,
                              std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (ids.empty())
                return ts_errc::parameter_invalid;

            return to_ts_errc(ts_funcs.requestChannelGroupDelPerm(
            connection_id, group_id, continue_on_error ? 1 : 0, ids.data(), ids.size(),
            return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_list(connection_id_t connection_id,
                          permission_group_id_t group_id,
                          std::string_view return_code /*= ""*/) -> std::error_code
        {
            return to_ts_errc(ts_funcs.requestChannelGroupPermList(
            connection_id, group_id, return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_set_client(connection_id_t connection_id,
                                gsl::span<Set_Client_Channel_Group_Entry> entries,
                                std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (entries.empty())
                return ts_errc::parameter_invalid;

            std::vector<permission_group_id_t> channel_group_ids;
            std::vector<channel_id_t> channel_ids;
            std::vector<client_db_id_t> client_db_ids;

            for (const auto &entry : entries)
            {
                channel_group_ids.push_back(entry.channel_group_id);
                channel_ids.push_back(entry.channel_id);
                client_db_ids.push_back(entry.client_db_id);
            }

            return to_ts_errc(ts_funcs.requestSetClientChannelGroup(
            connection_id, channel_group_ids.data(), channel_ids.data(), client_db_ids.data(),
            channel_group_ids.size(), return_code.empty() ? nullptr : return_code.data()));
        }

    }  // namespace channel_group

    namespace channel
    {
        auto request_add_perm(connection_id_t connection_id,
                              channel_id_t channel_id,
                              gsl::span<Permission_Entry> entries,
                              std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (entries.empty())
                return ts_errc::parameter_invalid;

            std::vector<permission_id_t> permission_ids;
            std::vector<int32_t> values;

            for (const auto &entry : entries)
            {
                permission_ids.push_back(entry.id);
                values.push_back(entry.value);
            }

            return to_ts_errc(ts_funcs.requestChannelAddPerm(
            connection_id, channel_id, permission_ids.data(), values.data(), permission_ids.size(),
            return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_del_perm(connection_id_t connection_id,
                              channel_id_t channel_id,
                              gsl::span<permission_id_t> ids,
                              std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (ids.empty())
                return ts_errc::parameter_invalid;

            return to_ts_errc(
            ts_funcs.requestChannelDelPerm(connection_id, channel_id, ids.data(), ids.size(),
                                           return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_list(connection_id_t connection_id,
                          channel_id_t channel_id,
                          std::string_view return_code /*= ""*/) -> std::error_code
        {
            return to_ts_errc(ts_funcs.requestChannelPermList(
            connection_id, channel_id, return_code.empty() ? nullptr : return_code.data()));
        }

    }  // namespace channel

    namespace client
    {
        auto request_add_perm(connection_id_t connection_id,
                              client_db_id_t client_db_id,
                              gsl::span<Permission_Entry> entries,
                              std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (entries.empty())
                return ts_errc::parameter_invalid;

            std::vector<permission_id_t> permission_ids;
            std::vector<int32_t> values;
            std::vector<int32_t> skip;

            for (const auto &entry : entries)
            {
                permission_ids.push_back(entry.id);
                values.push_back(entry.value);
                skip.push_back(entry.skip);
            }

            return to_ts_errc(ts_funcs.requestClientAddPerm(
            connection_id, client_db_id, permission_ids.data(), values.data(), skip.data(),
            permission_ids.size(), return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_del_perm(connection_id_t connection_id,
                              client_db_id_t client_db_id,
                              gsl::span<permission_id_t> ids,
                              std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (ids.empty())
                return ts_errc::parameter_invalid;

            return to_ts_errc(
            ts_funcs.requestClientDelPerm(connection_id, client_db_id, ids.data(), ids.size(),
                                          return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_list(connection_id_t connection_id,
                          client_db_id_t client_db_id,
                          std::string_view return_code /*= ""*/) -> std::error_code
        {
            return to_ts_errc(ts_funcs.requestClientPermList(
            connection_id, client_db_id, return_code.empty() ? nullptr : return_code.data()));
        }

    }  // namespace client

    namespace channel_client
    {
        auto request_add_perm(connection_id_t connection_id,
                              channel_id_t channel_id,
                              client_db_id_t client_db_id,
                              gsl::span<Permission_Entry> entries,
                              std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (entries.empty())
                return ts_errc::parameter_invalid;

            std::vector<permission_id_t> permission_ids;
            std::vector<int32_t> values;

            for (const auto &entry : entries)
            {
                permission_ids.push_back(entry.id);
                values.push_back(entry.value);
            }

            return to_ts_errc(ts_funcs.requestChannelClientAddPerm(
            connection_id, channel_id, client_db_id, permission_ids.data(), values.data(),
            permission_ids.size(), return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_del_perm(connection_id_t connection_id,
                              channel_id_t channel_id,
                              client_db_id_t client_db_id,
                              gsl::span<permission_id_t> ids,
                              std::string_view return_code /*= ""*/) -> std::error_code
        {
            if (ids.empty())
                return ts_errc::parameter_invalid;

            return to_ts_errc(ts_funcs.requestChannelClientDelPerm(
            connection_id, channel_id, client_db_id, ids.data(), ids.size(),
            return_code.empty() ? nullptr : return_code.data()));
        }

        auto request_list(connection_id_t connection_id,
                          channel_id_t channel_id,
                          client_db_id_t client_db_id,
                          std::string_view return_code /*= ""*/) -> std::error_code
        {
            return to_ts_errc(ts_funcs.requestChannelClientPermList(
            connection_id, channel_id, client_db_id, return_code.empty() ? nullptr : return_code.data()));
        }
    }  // namespace channel_client

    auto privilege_key_use(connection_id_t connection_id,
                           std::string_view token,
                           std::string_view return_code) -> std::error_code
    {
        return to_ts_errc(ts_funcs.privilegeKeyUse(connection_id, token.data(),
                                                   return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_permission_list(connection_id_t connection_id, std::string_view return_code)
    -> std::error_code
    {
        return to_ts_errc(
        ts_funcs.requestPermissionList(connection_id, return_code.empty() ? nullptr : return_code.data()));
    }

    auto request_permission_overview(connection_id_t connection_id,
                                     client_db_id_t client_db_id,
                                     channel_id_t channel_id,
                                     std::string_view return_code /*= ""*/) -> std::error_code
    {
        return to_ts_errc(ts_funcs.requestPermissionOverview(
        connection_id, client_db_id, channel_id, return_code.empty() ? nullptr : return_code.data()));
    }

}  // namespace permissions

/* Client functions */

auto get_app_path() -> std::string
{
    auto path_ = std::array<char, kPathBufferSize>{};
    // TODO we trust here that the string is terminated
    ts_funcs.getAppPath(path_.data(), kPathBufferSize);
    return {path_.data()};
}

auto get_resources_path() -> std::string
{
    auto path_ = std::array<char, kPathBufferSize>{};
    // TODO we trust here that the string is terminated
    ts_funcs.getResourcesPath(path_.data(), kPathBufferSize);
    return {path_.data()};
}

auto get_config_path() -> std::string
{
    auto path_ = std::array<char, kPathBufferSize>{};
    // TODO we trust here that the string is terminated
    ts_funcs.getConfigPath(path_.data(), kPathBufferSize);
    return {path_.data()};
}

auto get_plugin_path(std::string_view plugin_id) -> std::string
{
    auto path_ = std::array<char, kPathBufferSize>{};
    // TODO we trust here that the string is terminated
    ts_funcs.getPluginPath(path_.data(), kPathBufferSize, plugin_id.data());
    return {path_.data()};
}

auto get_current_server_connection_handler_id() -> connection_id_t
{
    return ts_funcs.getCurrentServerConnectionHandlerID();
}

void print_message(connection_id_t connection_id,
                   std::string_view message,
                   PluginMessageTarget message_target)
{
    ts_funcs.printMessage(connection_id, message.data(), message_target);
}

void print_message_to_current_tab(std::string_view message)
{
    ts_funcs.printMessageToCurrentTab(message.data());
}

auto urls_to_bb(std::string_view text, size_t max_len) -> std::string
{
    if (text.empty())
        return {};

    auto result_c = std::vector<char>();
    result_c.resize(max_len);
    ts_funcs.urlsToBB(text.data(), result_c.data(), max_len);
    return std::string(result_c.data());
}

void send_plugin_command(connection_id_t connection_id,
                         std::string_view plugin_id,
                         std::string_view command,
                         int target_mode,
                         gsl::span<const client_id_t> target_ids,
                         std::string_view return_code /*= ""*/)
{
    ts_funcs.sendPluginCommand(connection_id, plugin_id.data(), command.data(), target_mode,
                               target_ids.empty() ? nullptr : target_ids.data(),
                               return_code.empty() ? nullptr : return_code.data());
}

// TODO this is buggy in that it uses space as delimiter for the list entries
// given that this doesn't do anything TeamSpeak related(?!), I'm gonna call:
// deprecated
auto get_directories(std::string_view path, size_t max_len) -> std::string
{
    if (path.empty())
        return {};

    auto result_c = std::vector<char>();
    result_c.resize(max_len);
    ts_funcs.getDirectories(path.data(), result_c.data(), max_len);
    return std::string(result_c.data());
}

auto get_server_connect_info(connection_id_t connection_id)
-> std::tuple<std::error_code, std::string, uint16_t, std::string>
{
    auto host = std::array<char, kServerInfoBufferSize>();
    auto port = uint16_t{0};
    auto pw = std::array<char, kServerInfoBufferSize>();
    const auto error = to_ts_errc(
    ts_funcs.getServerConnectInfo(connection_id, host.data(), &port, pw.data(), kServerInfoBufferSize));

    if (host.back() != '\0')
        host.back() = '\0';

    if (pw.back() != '\0')
        pw.back() = '\0';

    return {error, host.empty() ? std::string{} : std::string{host.data()}, port,
            pw.empty() ? std::string{} : std::string{pw.data()}};
}

auto get_channel_connect_info(connection_id_t connection_id, channel_id_t channel_id)
-> std::tuple<std::error_code, std::string, std::string>
{
    auto path = std::array<char, kChannelInfoBufferSize>();
    auto pw = std::array<char, kChannelInfoBufferSize>();
    const auto error = to_ts_errc(ts_funcs.getChannelConnectInfo(connection_id, channel_id, path.data(),
                                                                 pw.data(), kChannelInfoBufferSize));

    if (path.back() != '\0')
        path.back() = '\0';

    if (pw.back() != '\0')
        pw.back() = '\0';

    return {error, path.empty() ? std::string{} : std::string{path.data()},
            pw.empty() ? std::string{} : std::string{pw.data()}};
}

auto create_return_code(std::string_view plugin_id) -> std::string
{
    auto result_c = std::array<char, kReturnCodeBufferSize>();
    ts_funcs.createReturnCode(plugin_id.data(), result_c.data(), kReturnCodeBufferSize);
    if (result_c.back() != '\0')
        result_c.back() = '\0';

    return std::string{result_c.data()};
}

auto request_info_update(connection_id_t connection_id, PluginItemType item_type, uint64_t item_id)
-> std::error_code
{
    return to_ts_errc(ts_funcs.requestInfoUpdate(connection_id, item_type, item_id));
}

auto get_server_version(connection_id_t connection_id) -> uint64_t
{
    return ts_funcs.getServerVersion(connection_id);
}

auto is_whispering(connection_id_t connection_id, client_id_t client_id) -> std::tuple<std::error_code, bool>
{
    int result = 0;
    const auto error = to_ts_errc(ts_funcs.isWhispering(connection_id, client_id, &result));
    return {error, !!result};
}

auto is_receiving_whisper(connection_id_t connection_id, client_id_t client_id)
-> std::tuple<std::error_code, bool>
{
    auto is_receiving_whisper = int32_t{0};
    const auto error =
    to_ts_errc(ts_funcs.isReceivingWhisper(connection_id, client_id, &is_receiving_whisper));
    return {error, !!is_receiving_whisper};
}

auto get_avatar(connection_id_t connection_id, client_id_t client_id)
-> std::tuple<std::error_code, std::string>
{
    auto result_c = std::array<char, kPathBufferSize>();
    const auto error =
    to_ts_errc(ts_funcs.getAvatar(connection_id, client_id, result_c.data(), kPathBufferSize));
    if (result_c.back() != '\0')
        result_c.back() = '\0';

    return {error, std::string{result_c.data()}};
}

void set_plugin_menu_enabled(std::string_view plugin_id, int32_t menu_id, bool enabled)
{
    ts_funcs.setPluginMenuEnabled(plugin_id.data(), menu_id, enabled ? 1 : 0);
}

void show_hotkey_setup()
{
    ts_funcs.showHotkeySetup();
}

void request_hotkey_input_dialog(std::string_view plugin_id,
                                 std::string_view keyword,
                                 bool is_down,
                                 void *q_parent_window)
{
    ts_funcs.requestHotkeyInputDialog(plugin_id.data(), keyword.data(), is_down ? 1 : 0, q_parent_window);
}

auto get_hotkey_from_keyword(std::string_view plugin_id,
                             gsl::span<std::string_view> keywords,
                             int32_t max_results) -> std::tuple<std::error_code, std::vector<std::string>>
{
    auto keywords_ = std::vector<const char *>{};
    for (const auto &keyword : keywords)
        keywords_.push_back(keyword.data());

    auto holder = std::vector<std::array<char, kHotkeyBufferSize>>();
    for (auto i = int32_t{0}; i < max_results; ++i)
        holder.push_back({});

    auto result_c = std::vector<char *>();
    for (auto &&holded : holder)
        result_c.push_back(holded.data());

    const auto error = to_ts_errc(ts_funcs.getHotkeyFromKeyword(
    plugin_id.data(), keywords_.data(), result_c.data(), keywords_.size(), kHotkeyBufferSize));

    auto result = std::vector<std::string>{};
    if (ts_errc::ok == error)
    {
        for (const auto &charray : holder)
            result.emplace_back(std::string(charray.data()));
    }
    return {error, result};
}

auto get_client_display_name(connection_id_t connection_id, client_id_t client_id)
-> std::tuple<std::error_code, std::string>
{
    auto result = std::array<char, kDisplayNameBufferSize>{};
    const auto error =
    to_ts_errc(ts_funcs.getClientDisplayName(connection_id, client_id, result.data(), result.size() - 1));
    return {error, {result.data()}};
}

auto get_bookmark_list() -> std::tuple<std::error_code, PluginBookmarkList *>
{
    struct PluginBookmarkList *list = nullptr;
    const auto error = to_ts_errc(ts_funcs.getBookmarkList(&list));
    return {error, list};
}

void free_bookmark_list(PluginBookmarkList *list)
{
    auto items = gsl::span<PluginBookmarkItem>{&list->items[0], gsl::narrow_cast<size_t>(list->itemcount)};
    for (const auto &item : items)
    {
        if (item.isFolder)
        {
            free_bookmark_list(item.folder);
            ts_funcs.freeMemory(item.name);
        }
        else
        {
            ts_funcs.freeMemory(item.name);
            ts_funcs.freeMemory(item.uuid);
        }
    }
    ts_funcs.freeMemory(list);
}

auto get_profile_list(PluginGuiProfile profile) -> std::tuple<std::error_code, int, std::vector<std::string>>
{
    std::vector<std::string> result;
    char **profiles = nullptr;
    int default_profile = 0;
    const auto error = to_ts_errc(ts_funcs.getProfileList(profile, &default_profile, &profiles));
    if (ts_errc::ok == error)
    {
        for (int i = 0; profiles[i]; ++i)
        {
            result.emplace_back(profiles[i]);
            ts_funcs.freeMemory(profiles[i]);
        }
        ts_funcs.freeMemory(profiles);
    }
    return {error, default_profile, result};
}

auto gui_connect(PluginConnectTab connect_tab,
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
                 std::string_view phonetic_name) -> std::tuple<std::error_code, connection_id_t>
{
    auto result = connection_id_t{0};
    const auto error = to_ts_errc(ts_funcs.guiConnect(
    connect_tab, server_label.data(), server_address.data(), server_pw.data(), nickname.data(),
    channel.data(), channel_pw.data(), capture_profile.data(), playback_profile.data(), hotkey_profile.data(),
    sound_profile.data(), user_identity.data(), one_time_key.data(), phonetic_name.data(), &result));
    return {error, result};
}

auto gui_connect_bookmark(PluginConnectTab connect_tab, std::string_view bookmark_uuid)
-> std::tuple<std::error_code, connection_id_t>
{
    auto result = connection_id_t{0};
    const auto error = to_ts_errc(ts_funcs.guiConnectBookmark(connect_tab, bookmark_uuid.data(), &result));
    return {error, result};
}

auto create_bookmark(std::string_view bookmark_uuid,
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
                     std::string_view phonetic_name) -> std::error_code
{
    // TODO how would an addon create a valid new uuid?
    if (bookmark_uuid.empty())
        return ts_errc::parameter_invalid;

    if (server_label.empty())
        return ts_errc::parameter_invalid;

    if (server_address.empty())
        return ts_errc::parameter_invalid;

    if (nickname.empty())
        return ts_errc::parameter_invalid;

    return to_ts_errc(ts_funcs.createBookmark(
    bookmark_uuid.data(), server_label.data(), server_address.data(),
    server_pw.empty() ? nullptr : server_pw.data(), nickname.data(),
    channel.empty() ? nullptr : channel.data(), channel_pw.empty() ? nullptr : channel_pw.data(),
    capture_profile.empty() ? nullptr : capture_profile.data(),
    playback_profile.empty() ? nullptr : playback_profile.data(),
    hotkey_profile.empty() ? nullptr : hotkey_profile.data(),
    sound_profile.empty() ? nullptr : sound_profile.data(), client_uid.empty() ? nullptr : client_uid.data(),
    one_time_key.empty() ? nullptr : one_time_key.data(),
    phonetic_name.empty() ? nullptr : phonetic_name.data()));
}

auto get_permission_id_by_name(connection_id_t connection_id, std::string_view permissionName)
-> std::tuple<std::error_code, uint32_t>
{
    auto result = uint32_t{0};
    const auto error =
    to_ts_errc(ts_funcs.getPermissionIDByName(connection_id, permissionName.data(), &result));
    return {error, result};
}

auto get_client_needed_permission(connection_id_t connection_id, std::string_view permissionName)
-> std::tuple<std::error_code, int32_t>
{
    auto result = int32_t{0};
    const auto error =
    to_ts_errc(ts_funcs.getClientNeededPermission(connection_id, permissionName.data(), &result));
    return {error, result};
}

void notify_key_event(std::string_view pluginID, std::string_view keyIdentifier, bool up_down)
{
    ts_funcs.notifyKeyEvent(pluginID.data(), keyIdentifier.data(), up_down ? 1 : 0);
}

auto start_recording(connection_id_t connection_id,
                     bool multitrack,
                     bool noFileSelector,
                     std::string_view path) -> std::error_code
{
    return to_ts_errc(
    ts_funcs.startRecording(connection_id, multitrack ? 1 : 0, noFileSelector ? 1 : 0, path.data()));
}

auto stop_recording(connection_id_t connection_id) -> std::error_code
{
    return to_ts_errc(ts_funcs.stopRecording(connection_id));
}

/* Convenience functions */

auto request_clients_move(connection_id_t connection_id,
                          gsl::span<client_id_t> target_client_ids,
                          channel_id_t new_channel_id,
                          std::string_view password,
                          std::string_view return_code /*= ""*/) -> std::error_code
{
    if (target_client_ids.empty())
        return ts_errc::parameter_invalid;

    auto target_client_ids_ = std::vector<client_id_t>(target_client_ids.begin(), target_client_ids.end());
    if (target_client_ids_.back() != 0)
        target_client_ids_.push_back(0);

    return to_ts_errc(ts_funcs.requestClientsMove(connection_id, target_client_ids_.data(), new_channel_id,
                                                  password.empty() ? nullptr : password.data(),
                                                  return_code.empty() ? nullptr : return_code.data()));
}

auto request_clients_kick(connection_id_t connection_id,
                          gsl::span<client_id_t> target_client_ids,
                          Kick_From kick_from,
                          std::string_view reason,
                          std::string_view return_code /*= ""*/) -> std::error_code
{
    auto error = ts_errc::parameter_invalid;
    if (target_client_ids.empty())
        return error;

    auto target_client_ids_ = std::vector<client_id_t>(target_client_ids.begin(), target_client_ids.end());
    if (target_client_ids_.back() != 0)
        target_client_ids_.push_back(0);


    if (Kick_From::Server == kick_from)
    {
        error = to_ts_errc(ts_funcs.requestClientsKickFromServer(
        connection_id, target_client_ids_.data(), reason.empty() ? nullptr : reason.data(),
        return_code.empty() ? nullptr : return_code.data()));
    }
    else if (Kick_From::Channel == kick_from)
    {
        error = to_ts_errc(ts_funcs.requestClientsKickFromChannel(
        connection_id, target_client_ids_.data(), reason.empty() ? nullptr : reason.data(),
        return_code.empty() ? nullptr : return_code.data()));
    }
    return error;
}

/* Helpers */

auto get_client_property_name(size_t flag) -> std::tuple<std::error_code, std::string>
{
    std::string result;
    char *result_c = nullptr;
    const auto error = to_ts_errc(ts_funcs.clientPropertyFlagToString(flag, &result_c));
    if (ts_errc::ok == error)
    {
        result = std::string(result_c);
        ts_funcs.freeMemory(result_c);
    }
    return {error, result};
}

auto get_channel_property_name(size_t flag) -> std::tuple<std::error_code, std::string>
{
    std::string result;
    char *result_c = nullptr;
    const auto error = to_ts_errc(ts_funcs.channelPropertyFlagToString(flag, &result_c));
    if (ts_errc::ok == error)
    {
        result = std::string(result_c);
        ts_funcs.freeMemory(result_c);
    }
    return {error, result};
}

auto get_server_property_name(size_t flag) -> std::tuple<std::error_code, std::string>
{
    std::string result;
    char *result_c = nullptr;
    const auto error = to_ts_errc(ts_funcs.serverPropertyFlagToString(flag, &result_c));
    if (ts_errc::ok == error)
    {
        result = std::string(result_c);
        ts_funcs.freeMemory(result_c);
    }
    return {error, result};
}

auto get_client_property_flag(std::string_view name) -> std::tuple<std::error_code, size_t>
{
    auto value = size_t{0};
    const auto error = to_ts_errc(ts_funcs.clientPropertyStringToFlag(name.data(), &value));
    return {error, (ts_errc::ok == error) ? value : 0};
}

auto get_channel_property_flag(std::string_view name) -> std::tuple<std::error_code, size_t>
{
    auto value = size_t{0};
    const auto error = to_ts_errc(ts_funcs.channelPropertyStringToFlag(name.data(), &value));
    return {error, (ts_errc::ok == error) ? value : 0};
}

auto get_server_property_flag(std::string_view name) -> std::tuple<std::error_code, size_t>
{
    auto value = size_t{0};
    const auto error = to_ts_errc(ts_funcs.serverPropertyStringToFlag(name.data(), &value));
    return {error, (ts_errc::ok == error) ? value : 0};
}

}  // namespace com::teamspeak::pluginsdk::funcs
