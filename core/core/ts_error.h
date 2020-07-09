#pragma once

#include <system_error>

// The idea here is: the values are 2 bytes wide, the first byte identifies the group, the second the count
// within that group

enum class ts_errc
{
    // general
    ok = 0x0000,
    undefined = 0x0001,
    not_implemented = 0x0002,
    ok_no_update = 0x0003,
    dont_notify = 0x0004,
    lib_time_limit_reached = 0x0005,
    out_of_memory = 0x0006,
    canceled = 0x0007,

    // dunno
    command_not_found = 0x0100,
    unable_to_bind_network_port = 0x0101,
    no_network_port_available = 0x0102,
    port_already_in_use = 0x0103,

    // client
    client_invalid_id = 0x0200,
    client_nickname_inuse = 0x0201,
    client_protocol_limit_reached = 0x0203,
    client_invalid_type = 0x0204,
    client_already_subscribed = 0x0205,
    client_not_logged_in = 0x0206,
    client_could_not_validate_identity = 0x0207,
    client_version_outdated = 0x020a,
    client_is_flooding = 0x020c,
    client_hacked = 0x020d,
    client_cannot_verify_now = 0x020e,
    client_login_not_permitted = 0x020f,
    client_not_subscribed = 0x0210,

    // channel
    channel_invalid_id = 0x0300,
    channel_protocol_limit_reached = 0x0301,
    channel_already_in = 0x0302,
    channel_name_inuse = 0x0303,
    channel_not_empty = 0x0304,
    channel_can_not_delete_default = 0x0305,
    channel_default_require_permanent = 0x0306,
    channel_invalid_flags = 0x0307,
    channel_parent_not_permanent = 0x0308,
    channel_maxclients_reached = 0x0309,
    channel_maxfamily_reached = 0x030a,
    channel_invalid_order = 0x030b,
    channel_no_filetransfer_supported = 0x030c,
    channel_invalid_password = 0x030d,
    channel_invalid_security_hash = 0x030f,  // note 0x030e is defined in public_rare_errors;

    // server
    server_invalid_id = 0x0400,
    server_running = 0x0401,
    server_is_shutting_down = 0x0402,
    server_maxclients_reached = 0x0403,
    server_invalid_password = 0x0404,
    server_is_virtual = 0x0407,
    server_is_not_running = 0x0409,
    server_is_booting = 0x040a,
    server_status_invalid = 0x040b,
    server_version_outdated = 0x040d,
    server_duplicate_running = 0x040e,

    // parameter
    parameter_quote = 0x0600,
    parameter_invalid_count = 0x0601,
    parameter_invalid = 0x0602,
    parameter_not_found = 0x0603,
    parameter_convert = 0x0604,
    parameter_invalid_size = 0x0605,
    parameter_missing = 0x0606,
    parameter_checksum = 0x0607,

    // unsorted, need further investigation
    vs_critical = 0x0700,
    connection_lost = 0x0701,
    not_connected = 0x0702,
    no_cached_connection_info = 0x0703,
    currently_not_possible = 0x0704,
    failed_connection_initialisation = 0x0705,
    could_not_resolve_hostname = 0x0706,
    invalid_server_connection_handler_id = 0x0707,
    could_not_initialise_input_manager = 0x0708,
    clientlibrary_not_initialised = 0x0709,
    serverlibrary_not_initialised = 0x070a,
    whisper_too_many_targets = 0x070b,
    whisper_no_targets = 0x070c,
    connection_ip_protocol_missing = 0x070d,
    // reserved                                   = 0x070e,
    illegal_server_license = 0x070f,

    // file transfer
    file_invalid_name = 0x0800,
    file_invalid_permissions = 0x0801,
    file_already_exists = 0x0802,
    file_not_found = 0x0803,
    file_io_error = 0x0804,
    file_invalid_transfer_id = 0x0805,
    file_invalid_path = 0x0806,
    file_no_files_available = 0x0807,
    file_overwrite_excludes_resume = 0x0808,
    file_invalid_size = 0x0809,
    file_already_in_use = 0x080a,
    file_could_not_open_connection = 0x080b,
    file_no_space_left_on_device = 0x080c,
    file_exceeds_file_system_maximum_size = 0x080d,
    file_transfer_connection_timeout = 0x080e,
    file_connection_lost = 0x080f,
    file_exceeds_supplied_size = 0x0810,
    file_transfer_complete = 0x0811,
    file_transfer_canceled = 0x0812,
    file_transfer_interrupted = 0x0813,
    file_transfer_server_quota_exceeded = 0x0814,
    file_transfer_client_quota_exceeded = 0x0815,
    file_transfer_reset = 0x0816,
    file_transfer_limit_reached = 0x0817,

    // sound
    sound_preprocessor_disabled = 0x0900,
    sound_internal_preprocessor = 0x0901,
    sound_internal_encoder = 0x0902,
    sound_internal_playback = 0x0903,
    sound_no_capture_device_available = 0x0904,
    sound_no_playback_device_available = 0x0905,
    sound_could_not_open_capture_device = 0x0906,
    sound_could_not_open_playback_device = 0x0907,
    sound_handler_has_device = 0x0908,
    sound_invalid_capture_device = 0x0909,
    sound_invalid_playback_device = 0x090a,
    sound_invalid_wave = 0x090b,
    sound_unsupported_wave = 0x090c,
    sound_open_wave = 0x090d,
    sound_internal_capture = 0x090e,
    sound_device_in_use = 0x090f,
    sound_device_already_registerred = 0x0910,
    sound_unknown_device = 0x0911,
    sound_unsupported_frequency = 0x0912,
    sound_invalid_channel_count = 0x0913,
    sound_read_wave = 0x0914,
    sound_need_more_data = 0x0915,  // for internal purposes only
    sound_device_busy = 0x0916,     // for internal purposes only
    sound_no_data = 0x0917,
    sound_channel_mask_mismatch = 0x0918,


    // permissions
    permissions_client_insufficient = 0x0a08,
    permissions = 0x0a0c,

    // accounting
    accounting_virtualserver_limit_reached = 0x0b00,
    accounting_slot_limit_reached = 0x0b01,
    accounting_license_file_not_found = 0x0b02,
    accounting_license_date_not_ok = 0x0b03,
    accounting_unable_to_connect_to_server = 0x0b04,
    accounting_unknown_error = 0x0b05,
    accounting_server_error = 0x0b06,
    accounting_instance_limit_reached = 0x0b07,
    accounting_instance_check_error = 0x0b08,
    accounting_license_file_invalid = 0x0b09,
    accounting_running_elsewhere = 0x0b0a,
    accounting_instance_duplicated = 0x0b0b,
    accounting_already_started = 0x0b0c,
    accounting_not_started = 0x0b0d,
    accounting_to_many_starts = 0x0b0e,

    // provisioning server
    provisioning_invalid_password = 0x1100,
    provisioning_invalid_request = 0x1101,
    provisioning_no_slots_available = 0x1102,
    provisioning_pool_missing = 0x1103,
    provisioning_pool_unknown = 0x1104,
    provisioning_unknown_ip_location = 0x1105,
    provisioning_internal_tries_exceeded = 0x1106,
    provisioning_too_many_slots_requested = 0x1107,
    provisioning_too_many_reserved = 0x1108,
    provisioning_could_not_connect = 0x1109,
    provisioning_auth_server_not_connected = 0x1110,
    provisioning_auth_data_too_large = 0x1111,
    provisioning_already_initialized = 0x1112,
    provisioning_not_initialized = 0x1113,
    provisioning_connecting = 0x1114,
    provisioning_already_connected = 0x1115,
    provisioning_not_connected = 0x1116,
    provisioning_io_error = 0x1117,
    provisioning_invalid_timeout = 0x1118,
    provisioning_ts3server_not_found = 0x1119,
    provisioning_no_permission = 0x111A,

#ifndef SDK
    // client
    client_invalid_password = 0x0208,
    client_too_many_clones_connected = 0x0209,
    client_is_online = 0x020b,

    // channel
    channel_is_private_channel = 0x030e,
    // note 0x030f is defined in public_errors;

    // database
    database = 0x0500,
    database_empty_result = 0x0501,
    database_duplicate_entry = 0x0502,
    database_no_modifications = 0x0503,
    database_constraint = 0x0504,
    database_reinvoke = 0x0505,

    // permissions
    permission_invalid_group_id = 0x0a00,
    permission_duplicate_entry = 0x0a01,
    permission_invalid_perm_id = 0x0a02,
    permission_empty_result = 0x0a03,
    permission_default_group_forbidden = 0x0a04,
    permission_invalid_size = 0x0a05,
    permission_invalid_value = 0x0a06,
    permissions_group_not_empty = 0x0a07,
    permissions_insufficient_group_power = 0x0a09,
    permissions_insufficient_permission_power = 0x0a0a,
    permission_template_group_is_used = 0x0a0b,
    // 0x0a0c is in public_errors.h
    permission_used_by_integration = 0x0a0d,

    // server
    server_deployment_active = 0x0405,
    server_unable_to_stop_own_server = 0x0406,
    server_wrong_machineid = 0x0408,
    server_modal_quit = 0x040c,
    server_time_difference_too_large = 0x040f,
    server_blacklisted = 0x0410,
    server_shutdown = 0x0411,

    // messages
    message_invalid_id = 0x0c00,

    // ban
    ban_invalid_id = 0x0d00,
    connect_failed_banned = 0x0d01,
    rename_failed_banned = 0x0d02,
    ban_flooding = 0x0d03,

    // tts
    tts_unable_to_initialize = 0x0e00,

    // privilege key
    privilege_key_invalid = 0x0f00,

    // voip
    voip_pjsua = 0x1000,
    voip_already_initialized = 0x1001,
    voip_too_many_accounts = 0x1002,
    voip_invalid_account = 0x1003,
    voip_internal_error = 0x1004,
    voip_invalid_connectionId = 0x1005,
    voip_cannot_answer_initiated_call = 0x1006,
    voip_not_initialized = 0x1007,

    // ed25519
    ed25519_rng_fail = 0x1300,
    ed25519_signature_invalid = 0x1301,
    ed25519_invalid_key = 0x1302,
    ed25519_unable_to_create_valid_key = 0x1303,
    ed25519_out_of_memory = 0x1304,
    ed25519_exists = 0x1305,
    ed25519_read_beyond_eof = 0x1306,
    ed25519_write_beyond_eof = 0x1307,
    ed25519_version = 0x1308,
    ed25519_invalid = 0x1309,
    ed25519_invalid_date = 0x130A,
    ed25519_unauthorized = 0x130B,
    ed25519_invalid_type = 0x130C,
    ed25519_address_nomatch = 0x130D,
    ed25519_not_valid_yet = 0x130E,
    ed25519_expired = 0x130F,
    ed25519_index_out_of_range = 0x1310,
    ed25519_invalid_size = 0x1311,

    // mytsid - client
    invalid_mytsid_data = 0x1200,
    invalid_integration = 0x1201,
#endif
};

namespace com::teamspeak
{
ts_errc to_ts_errc(uint32_t err);
}

namespace std
{
template <> struct is_error_code_enum<ts_errc> : true_type
{
};
}  // namespace std

std::error_code make_error_code(ts_errc);
