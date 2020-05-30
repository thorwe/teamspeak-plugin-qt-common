#pragma once

#include <utility>
#include <cstdint>
#include <string>
#include <vector>
#include <tuple>

#include "teamspeak/public_definitions.h"

class Connection_Base
{
public:
	Connection_Base(std::uint64_t server_connection_id);

	const std::uint64_t id;

	std::pair<std::uint32_t, std::string> unique_identifier() const;  // can be used to identify this particular server installation
	std::pair<std::uint32_t, std::string> name() const;
	std::pair<std::uint32_t, std::string> platform() const; // ToDo: is this a string?
	std::pair<std::uint32_t, std::string> version() const; // ToDo: is this a string?
	std::pair<std::uint32_t, std::string> created() const; // ToDo: is this a string?

	enum class Codec_Encryption_Mode : uint8_t {
		Per_Channel,  // Default
		Forced_Off,
		Forced_On
	};
	std::pair<std::uint32_t, Codec_Encryption_Mode> codec_encryption_mode() const;

	// only available on request (-> update_variables)
	std::pair<std::uint32_t, std::string>	welcome_message();
	std::pair<std::uint32_t, std::uint64_t> max_clients();
	std::pair<std::uint32_t, std::uint64_t> clients_online();
	std::pair<std::uint32_t, std::uint64_t> channels_online();
	std::pair<std::uint32_t, std::string>	uptime();

	/* Access clientlib information */

	/* Query own client ID */
	std::pair<std::uint32_t, std::uint16_t> get_client_id();
	
	/* Server info */
	std::pair<std::uint32_t, std::int32_t>	get_variable_as_int(std::size_t virtual_server_property) const;
	std::pair<std::uint32_t, std::uint64_t>	get_variable_as_uint64(std::size_t virtual_server_property) const;
	std::pair<std::uint32_t, std::string>	get_variable_as_string(std::size_t virtual_server_property) const;
	std::uint32_t request_variables();

	/* Connection info */
	enum class Connection_Status : uint8_t {
		Disconnected,				//There is no activity to the server, this is the default value
		Connecting,					//We are trying to connect, we haven't got a clientID yet, we haven't been accepted by the server
		Connected,					//The server has accepted us, we can talk and hear and we got a clientID, but we don't have the channels and clients yet, we can get server infos (welcome msg etc.)
		Connection_Establishing,	//we are CONNECTED and we are visible
		Connection_Established,		//we are CONNECTED and we have the client and channels available
	};
	std::pair<std::uint32_t, Connection_Status> connection_status();
	
	/* Plugin related */
	std::uint32_t request_send_client_query_command(const std::string& command, const std::string& return_code = std::string());
	
	/* Filetransfer */
	std::pair<std::uint32_t, std::uint16_t> send_file(std::uint64_t channel_id, const std::string& channel_pw, const std::string& file, bool overwrite, bool resume, const std::string& source_directory, const std::string& return_code = std::string());
	std::pair<std::uint32_t, std::uint16_t> request_file(std::uint64_t channel_id, const std::string& channel_pw, const std::string& file, bool overwrite, bool resume, const std::string& destination_directory, const std::string& return_code = std::string());
	std::uint32_t halt_transfer(std::uint16_t transfer_id, bool delete_unfinished_file, const std::string& return_code = std::string());
	std::uint32_t request_file_list(std::uint64_t channel_id, const std::string& channel_pw, const std::string& path, const std::string& return_code = std::string());
	std::uint32_t request_file_info(std::uint64_t channel_id, const std::string& channel_pw, const std::string& file, const std::string& return_code = std::string());
	std::uint32_t request_delete_file(std::uint64_t channel_id, const std::string& channel_pw, const char** file, const std::string& return_code = std::string());
	std::uint32_t request_create_directory(std::uint64_t channel_id, const std::string& channel_pw, const std::string& directory_path, const std::string& return_code = std::string());
	std::uint32_t request_rename_file(std::uint64_t from_channel_id, const std::string& channel_pw, std::uint64_t to_channel_id, const std::string& to_channel_pw, const std::string& old_file, const std::string& new_file, const std::string& return_code = std::string());

	/* Offline message management */
	std::uint32_t request_message_add(const std::string& to_client_uid, const std::string& subject, const std::string& message, const std::string& return_code = std::string());
	std::uint32_t request_message_del(std::uint64_t message_id, const std::string& return_code = std::string());
	std::uint32_t request_message_get(std::uint64_t message_id, const std::string& return_code = std::string());
	std::uint32_t request_message_list(const std::string& return_code = std::string());
	std::uint32_t request_message_update_flag(std::uint64_t message_id, int flag, const std::string& return_code = std::string());

	/* Interacting with the server - confirming passwords */
	std::uint32_t verify_server_password(const std::string& server_pw, const std::string& return_code = std::string());
	std::uint32_t verify_channel_password(std::uint64_t channel_id, const std::string& channel_pw, const std::string& return_code = std::string());

	/* Interacting with the server - banning */
	std::uint32_t ban_client(std::uint16_t client_id, std::uint64_t time_in_seconds, const std::string& ban_reason, const std::string& return_code = std::string());
	std::uint32_t ban_add(const std::string& ip_regexp, const std::string& name_regexp, const std::string& unique_id, std::uint64_t time_in_seconds, const std::string& ban_reason, const std::string& return_code = std::string());
	std::uint32_t ban_clientdbid(std::uint64_t client_db_id, std::uint64_t time_in_seconds, const std::string& ban_reason, const std::string& return_code = std::string());
	std::uint32_t ban_del(std::uint64_t ban_id, const std::string& return_code = std::string());
	std::uint32_t ban_delall(const std::string& return_code = std::string());
	std::uint32_t request_ban_list(const std::string& return_code = std::string());

	/* Interacting with the server - complain */
	std::uint32_t request_complain_add(std::uint64_t target_client_db_id, const std::string& complain_reason, const std::string& return_code = std::string());
	std::uint32_t request_complain_del(std::uint64_t target_client_db_id, std::uint64_t from_client_db_id, const std::string& return_code = std::string());
	std::uint32_t request_complain_del_all(std::uint64_t target_client_db_id, const std::string& return_code = std::string());
	std::uint32_t request_complain_list(std::uint64_t target_client_db_id, const std::string& return_code = std::string());

	/* Permissions */
	std::uint32_t request_server_group_list(const std::string& return_code = std::string());
	std::uint32_t request_server_group_add(const std::string& group_name, int group_type, const std::string& return_code = std::string());
	std::uint32_t request_server_group_del(std::uint64_t server_group_id, int force, const std::string& return_code = std::string());
	std::uint32_t request_server_group_add_client(std::uint64_t server_group_id, std::uint64_t client_db_id, const std::string& return_code = std::string());
	std::uint32_t request_server_group_del_client(std::uint64_t server_group_id, std::uint64_t client_db_id, const std::string& return_code = std::string());
	std::uint32_t request_server_groups_by_client_id(std::uint64_t client_db_id, const std::string& return_code = std::string());
	std::uint32_t request_server_group_add_perm(std::uint64_t server_group_id, int continue_on_error, const unsigned int* permission_ids, const int* permission_values, const int* permission_negateds, const int* permission_skips, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_server_group_del_perm(std::uint64_t server_group_id, int continue_on_error, const unsigned int* permission_ids, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_server_group_perm_list(std::uint64_t server_group_id, const std::string& return_code = std::string());
	std::uint32_t request_server_group_client_list(std::uint64_t server_group_id, int with_names, const std::string& return_code = std::string());

	std::uint32_t request_channel_group_list(const std::string& return_code = std::string());
	std::uint32_t request_channel_group_add(const std::string& group_name, int group_type, const std::string& return_code = std::string());
	std::uint32_t request_channel_group_del(std::uint64_t channel_group_id, int force, const std::string& return_code = std::string());
	std::uint32_t request_channel_group_add_perm(std::uint64_t channel_group_id, int continue_on_error, const unsigned int* permission_ids, const int* permission_values, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_channel_group_del_perm(std::uint64_t channel_group_id, int continue_on_error, const unsigned int* permission_ids, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_channel_group_perm_list(std::uint64_t channel_group_id, const std::string& return_code = std::string());

	std::uint32_t request_set_client_channel_group(const std::uint64_t* channel_group_ids, const std::uint64_t* channel_ids, const std::uint64_t* client_db_ids, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_channel_add_perm(std::uint64_t channel_id, const unsigned int* permission_ids, const int* permission_values, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_channel_del_perm(std::uint64_t channel_id, const unsigned int* permission_ids, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_channel_perm_list(std::uint64_t channel_id, const std::string& return_code = std::string());

	std::uint32_t request_client_add_perm(std::uint64_t client_db_id, const unsigned int* permission_ids, const int* permission_values, const int* permission_skips, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_client_del_perm(std::uint64_t client_db_id, const unsigned int* permission_ids, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_client_perm_list(std::uint64_t client_db_id, const std::string& return_code = std::string());

	std::uint32_t request_channel_client_add_perm(std::uint64_t channel_id, std::uint64_t client_db_id, const unsigned int* permission_ids, const int* permission_values, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_channel_client_del_perm(std::uint64_t channel_id, std::uint64_t client_db_id, const unsigned int* permission_ids, int array_size, const std::string& return_code = std::string());
	std::uint32_t request_channel_client_perm_list(std::uint64_t channel_id, std::uint64_t client_db_id, const std::string& return_code = std::string());

	std::uint32_t privilege_key_use(const std::string& token_key, const std::string& return_code = std::string());
	std::uint32_t request_permission_list(const std::string& return_code = std::string());
	std::uint32_t request_permission_overview(std::uint64_t client_db_id, std::uint64_t channel_id, const std::string& return_code = std::string());

	/* Client functions */
	void print_message(const std::string& message, enum PluginMessageTarget message_target);
	
	enum class Plugin_Target_Mode : std::uint8_t {
		PluginCommandTarget_CURRENT_CHANNEL,					//send plugincmd to all clients in current channel
		PluginCommandTarget_SERVER,                             //send plugincmd to all clients on server
		PluginCommandTarget_CLIENT,                             //send plugincmd to all given client ids
		PluginCommandTarget_CURRENT_CHANNEL_SUBSCRIBED_CLIENTS  //send plugincmd to all subscribed clients in current channel
	};
	void send_plugin_command(const std::string& plugin_id, const std::string& command, Plugin_Target_Mode target_mode, const std::uint16_t* target_ids, const std::string& return_code = std::string());
	
	std::uint32_t get_server_connect_info(char* host, unsigned short* port, char* password, size_t max_len);
	std::uint32_t get_channel_connect_info(std::uint64_t channel_id, char* path, char* password, size_t max_len);

	std::uint32_t request_info_update(enum PluginItemType item_type, std::uint64_t item_id);
	std::uint64_t get_server_version();

	std::pair<std::uint32_t, bool> is_whispering(std::uint16_t client_id);
	std::pair<std::uint32_t, bool> is_receiving_whisper(std::uint16_t client_id);

	std::uint32_t get_avatar(std::uint16_t client_id, char* result, size_t max_len);
	std::pair<std::uint32_t, std::uint32_t> get_permission_id_by_name(const std::string& permission_name);
	std::pair<std::uint32_t, int32_t> get_client_needed_permission(const std::string& permission_name);

	/* Interaction with the server */

	std::uint32_t request_client_move(std::uint16_t client_id, std::uint64_t channel_id, const std::string& password, const std::string& return_code = std::string());
	std::uint32_t request_client_variables(std::uint16_t client_id, const std::string& return_code = std::string());
	std::uint32_t request_client_kick_from_channel(std::uint16_t client_id, const std::string& kick_reason, const std::string& return_code = std::string());
	std::uint32_t request_client_kick_from_server(std::uint16_t client_id, const std::string& kick_reason, const std::string& return_code = std::string());

	std::uint32_t request_channel_delete(std::uint64_t channel_id, int force, const std::string& return_code = std::string());
	std::uint32_t request_channel_move(std::uint64_t channel_id, std::uint64_t new_channel_parent_id, std::uint64_t new_channel_order, const std::string& return_code = std::string());

	std::uint32_t request_send_client_text_msg(const std::string& message, std::uint16_t client_id, const std::string& return_code = std::string());
	std::uint32_t request_send_channel_text_msg(const std::string& message, std::uint64_t channel_id, const std::string& return_code = std::string());
	std::uint32_t request_send_server_text_msg(const std::string& message, const std::string& return_code = std::string());

	std::uint32_t request_connection_info(std::uint16_t client_id, const std::string& return_code = std::string());
	std::uint32_t request_client_set_whisper_list(std::uint16_t client_id, std::vector<std::uint64_t>* target_channel_ids = nullptr, std::vector<std::uint16_t>* target_client_ids = nullptr, const std::string& return_code = std::string());

	std::uint32_t request_change_channel_subscriptions(bool subscribe, std::vector<std::uint64_t>& channel_ids, const std::string& return_code = std::string());
	std::uint32_t request_change_channel_subscriptions(bool subscribe, const std::string& return_code = std::string());
	std::uint32_t request_channel_description(std::uint64_t channel_id, const std::string& return_code = std::string());

	std::uint32_t request_clients_mute(bool mute, std::vector<std::uint16_t>& client_ids, const std::string& return_code = std::string());
	
	std::uint32_t request_client_poke(std::uint16_t client_id, const std::string& message, const std::string& return_code = std::string());
	std::uint32_t request_client_ids(const std::string& client_unique_id, const std::string& return_code = std::string());
	std::uint32_t client_chat_closed(const std::string& client_unique_id, std::uint16_t client_id, const std::string& return_code = std::string());
	std::uint32_t client_chat_composing(std::uint16_t client_id, const std::string& return_code = std::string());

	std::uint32_t request_server_temporary_password_add(const std::string& password, const std::string& description, std::uint64_t duration, std::uint64_t target_channel_id, const std::string& target_channel_pw, const std::string& return_code = std::string());
	std::uint32_t request_server_temporary_password_del(const std::string& password, const std::string& return_code = std::string());
	std::uint32_t request_server_temporary_password_list(const std::string& return_code = std::string());

	/* Sound */
	std::uint32_t open_device(const std::string& backend, bool playback, const std::string& device);
	std::tuple<std::uint32_t, std::string, bool> get_current_device_name(bool playback);
	std::pair<std::uint32_t, std::string> get_current_backend(bool playback);
	std::uint32_t initiate_graceful_playback_shutdown();
	std::uint32_t close_device(bool playback);
	std::uint32_t activate_capture_device();
	std::pair<std::uint32_t, std::uint64_t> play_wave_file_handle(const std::string& path, bool loop);
	std::uint32_t pause_wave_file_handle(std::uint64_t wave_handle, bool pause);
	std::uint32_t close_wave_file_handle(std::uint64_t wave_handle);
	std::uint32_t play_wave_file(const std::string& path);

	/* Preprocessor */
	enum class Pre_Processor_Info : std::uint8_t {
		decibel_last_period
	};
	std::pair<std::uint32_t, float> get_pre_processor_info(Pre_Processor_Info info);

	enum class Pre_Processor_Config : std::uint16_t {
		Name,
		Denoise,
		Voice_Activity_Detection,
		Voice_Activity_Detection_Level,
		Voice_Activity_Detection_Extrabuffersize,
		Automatic_Gain_Correction,
		Automatic_Gain_Correction_Level,
		Automatic_Gain_Correction_Max_Gain,
		Echo_Cancellation
	};
	std::pair<std::uint32_t, std::string> get_pre_processor_config_value(Pre_Processor_Config config);
	std::uint32_t set_pre_processor_config_value(Pre_Processor_Config config, const std::string& value);

	/* Encoder */
	enum class Encode_Config : std::uint8_t {
		Name,
		Quality,
		Bitrate
	};
	std::pair<std::uint32_t, std::string> get_encode_config(Encode_Config config);

	/* Playback */
	enum class Playback_Config : std::uint8_t {
		Voice_Volume_db,
		Wave_Volume_db
	};
	std::pair<std::uint32_t, float> get_playback_config(Playback_Config config);
	std::uint32_t set_playback_config(Playback_Config config, float val);

	std::uint32_t set_client_volume_modifier(std::uint16_t client_id, float value);

	/* Recording */
	std::uint32_t set_voice_recording(bool start);

	/* 3d sound positioning */
	std::uint32_t system_set_3D_listener_attributes(const TS3_VECTOR* position, const TS3_VECTOR* forward, const TS3_VECTOR* up);
	std::uint32_t set_3D_wave_attributes(std::uint64_t wave_handle, const TS3_VECTOR* position);
	std::uint32_t system_set_3D_settings(float distance_factor, float rolloff_scale);
	std::uint32_t channel_set_3D_attributes(std::uint16_t client_id, const TS3_VECTOR* position);
};
