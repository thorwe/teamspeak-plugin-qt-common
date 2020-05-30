#include "core/connection_base.h"

#include <QtCore/QString>

#include "teamspeak/public_errors.h"

#include "teamspeak/public_rare_definitions.h"

#include "ts_missing_definitions.h"

#include "ts3_functions.h"
#include "plugin.h"

Connection_Base::Connection_Base(uint64_t server_connection_id)
	: id(server_connection_id)
{
}

std::pair<uint32_t, std::string> Connection_Base::unique_identifier() const
{
	return get_variable_as_string(VIRTUALSERVER_UNIQUE_IDENTIFIER);
}

std::pair<uint32_t, std::string> Connection_Base::name() const
{
	return get_variable_as_string(VIRTUALSERVER_NAME);
}

std::pair<uint32_t, std::string> Connection_Base::platform() const
{
	return get_variable_as_string(VIRTUALSERVER_PLATFORM);
}

std::pair<uint32_t, std::string> Connection_Base::version() const
{
	return get_variable_as_string(VIRTUALSERVER_VERSION);
}

std::pair<uint32_t, std::string> Connection_Base::created() const
{
	return get_variable_as_string(VIRTUALSERVER_CREATED);
}

std::pair<uint32_t, Connection_Base::Codec_Encryption_Mode> Connection_Base::codec_encryption_mode() const
{
	const auto result = get_variable_as_int(VIRTUALSERVER_CODEC_ENCRYPTION_MODE);
	if (result.first != ERROR_ok)
		return { result.first, Codec_Encryption_Mode::Per_Channel };

	switch (result.second)
	{
	case CodecEncryptionMode::CODEC_ENCRYPTION_PER_CHANNEL:
		return { result.first, Codec_Encryption_Mode::Per_Channel };
	case CodecEncryptionMode::CODEC_ENCRYPTION_FORCED_OFF:
		return { result.first, Codec_Encryption_Mode::Forced_Off };
	case CodecEncryptionMode::CODEC_ENCRYPTION_FORCED_ON:
		return { result.first, Codec_Encryption_Mode::Forced_On };
	default:
		break;
	}
	return { ERROR_not_implemented, Codec_Encryption_Mode::Per_Channel };
}

std::pair<uint32_t, std::string> Connection_Base::welcome_message()
{
	return get_variable_as_string(VIRTUALSERVER_WELCOMEMESSAGE);
}

std::pair<uint32_t, uint64_t> Connection_Base::max_clients()
{
	return get_variable_as_uint64(VIRTUALSERVER_MAXCLIENTS);
}

std::pair<uint32_t, uint64_t> Connection_Base::clients_online()
{
	return get_variable_as_uint64(VIRTUALSERVER_CLIENTS_ONLINE);
}

std::pair<uint32_t, uint64_t> Connection_Base::channels_online()
{
	return get_variable_as_uint64(VIRTUALSERVER_CHANNELS_ONLINE);
}

std::pair<uint32_t, std::string> Connection_Base::uptime()
{
	return get_variable_as_string(VIRTUALSERVER_UPTIME);
}

std::pair<std::uint32_t, std::uint16_t> Connection_Base::get_client_id()
{
	std::uint16_t client_id;
	const auto error = ts3Functions.getClientID(id, &client_id);
	return { error, client_id };
}

std::pair<uint32_t, int32_t> Connection_Base::get_variable_as_int(std::size_t virtual_server_property) const
{
	int32_t val = 0;
	uint32_t error = ts3Functions.getServerVariableAsInt(id, virtual_server_property, &val);
	return { error, val };
}

std::pair<uint32_t, uint64_t> Connection_Base::get_variable_as_uint64(std::size_t virtual_server_property) const
{
	uint64_t val = 0;
	uint32_t error = ts3Functions.getServerVariableAsUInt64(id, virtual_server_property, &val);
	return { error, val };
}

std::pair<uint32_t, std::string> Connection_Base::get_variable_as_string(std::size_t virtual_server_property) const
{
	unsigned int error;
	char* s_val;
	if ((error = ts3Functions.getServerVariableAsString(id, virtual_server_property, &s_val)) != ERROR_ok)
		return { error, std::string() };

	auto val_q = QString::fromUtf8(s_val).toStdString();  // ToDo: replace
	ts3Functions.freeMemory(s_val);
	return { error, val_q };
}

uint32_t Connection_Base::request_variables()
{
	return ts3Functions.requestServerVariables(id);
}

std::pair<std::uint32_t, Connection_Base::Connection_Status> Connection_Base::connection_status()
{
	std::int32_t status{ STATUS_DISCONNECTED };
	Connection_Status conn_status{ Connection_Status::Disconnected };
	const auto error = ts3Functions.getConnectionStatus(id, &status);
	if (error == ERROR_ok)
	{
		switch (status)
		{
		case STATUS_DISCONNECTED:
			//conn_status = Connection_Status::DISCONNECTED;
			break;
		case STATUS_CONNECTING:
			conn_status = Connection_Status::Connecting;
			break;
		case STATUS_CONNECTED:
			conn_status = Connection_Status::Connected;
			break;
		case STATUS_CONNECTION_ESTABLISHING:
			conn_status = Connection_Status::Connection_Establishing;
			break;
		case STATUS_CONNECTION_ESTABLISHED:
			conn_status = Connection_Status::Connection_Established;
			break;
		default:
			break;
		}
	}
	return { error, conn_status };
}

std::uint32_t Connection_Base::request_send_client_query_command(const std::string& command, const std::string& return_code)
{
	return ts3Functions.requestSendClientQueryCommand(id, command.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::pair<std::uint32_t, std::uint16_t> Connection_Base::send_file(std::uint64_t channel_id, const std::string& channel_pw, const std::string& file, bool overwrite, bool resume, const std::string& source_directory, const std::string& return_code)
{
	std::uint16_t result{ 0 };
	const auto error = ts3Functions.sendFile(id, channel_id, channel_pw.c_str(), file.c_str(), overwrite, resume, source_directory.c_str(), &result, return_code.empty() ? nullptr : return_code.c_str());
	return { error, result };
}

std::pair<std::uint32_t, std::uint16_t> Connection_Base::request_file(std::uint64_t channel_id, const std::string& channel_pw, const std::string& file, bool overwrite, bool resume, const std::string& destination_directory, const std::string& return_code)
{
	std::uint16_t result{ 0 };
	const auto error = ts3Functions.requestFile(id, channel_id, channel_pw.c_str(), file.c_str(), overwrite, resume, destination_directory.c_str(), &result, return_code.empty() ? nullptr : return_code.c_str());
	return { error, result };
}

std::uint32_t Connection_Base::halt_transfer(std::uint16_t transfer_id, bool delete_unfinished_file, const std::string& return_code)
{
	return ts3Functions.haltTransfer(id, transfer_id, delete_unfinished_file, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_file_list(std::uint64_t channel_id, const std::string& channel_pw, const std::string& path, const std::string& return_code)
{
	return ts3Functions.requestFileList(id, channel_id, channel_pw.c_str(), path.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_file_info(std::uint64_t channel_id, const std::string& channel_pw, const std::string& file, const std::string& return_code)
{
	return ts3Functions.requestFileInfo(id, channel_id, channel_pw.c_str(), file.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_delete_file(std::uint64_t channel_id, const std::string& channel_pw, const char** file, const std::string& return_code)
{
	return ts3Functions.requestDeleteFile(id, channel_id, channel_pw.c_str(), file, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_create_directory(std::uint64_t channel_id, const std::string& channel_pw, const std::string& directory_path, const std::string& return_code)
{
	return ts3Functions.requestCreateDirectory(id, channel_id, channel_pw.c_str(), directory_path.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_rename_file(std::uint64_t from_channel_id, const std::string& channel_pw, std::uint64_t to_channel_id, const std::string& to_channel_pw, const std::string& old_file, const std::string& new_file, const std::string& return_code)
{
	return ts3Functions.requestRenameFile(id, from_channel_id, channel_pw.c_str(), to_channel_id, to_channel_pw.c_str(), old_file.c_str(), new_file.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_message_add(const std::string& to_client_uid, const std::string& subject, const std::string& message, const std::string& return_code)
{
	return ts3Functions.requestMessageAdd(id, to_client_uid.c_str(), subject.c_str(), message.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_message_del(std::uint64_t message_id, const std::string& return_code)
{
	return ts3Functions.requestMessageDel(id, message_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_message_get(std::uint64_t message_id, const std::string& return_code)
{
	return ts3Functions.requestMessageGet(id, message_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_message_list(const std::string& return_code)
{
	return ts3Functions.requestMessageList(id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_message_update_flag(std::uint64_t message_id, int flag, const std::string& return_code)
{
	return ts3Functions.requestMessageUpdateFlag(id, message_id, flag, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::verify_server_password(const std::string& server_pw, const std::string& return_code)
{
	return ts3Functions.verifyServerPassword(id, server_pw.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::verify_channel_password(std::uint64_t channel_id, const std::string& channel_pw, const std::string& return_code)
{
	return ts3Functions.verifyChannelPassword(id, channel_id, channel_pw.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::ban_client(std::uint16_t client_id, std::uint64_t time_in_seconds, const std::string& ban_reason, const std::string& return_code)
{
	return ts3Functions.banclient(id, client_id, time_in_seconds, ban_reason.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::ban_add(const std::string& ip_regexp, const std::string& name_regexp, const std::string& unique_id, std::uint64_t time_in_seconds, const std::string& ban_reason, const std::string& return_code)
{
	return ts3Functions.banadd(id, ip_regexp.c_str(), name_regexp.c_str(), unique_id.c_str(), time_in_seconds, ban_reason.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::ban_clientdbid(std::uint64_t client_db_id, std::uint64_t time_in_seconds, const std::string& ban_reason, const std::string& return_code)
{
	return ts3Functions.banclientdbid(id, client_db_id, time_in_seconds, ban_reason.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::ban_del(std::uint64_t ban_id, const std::string& return_code)
{
	return ts3Functions.bandel(id, ban_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::ban_delall(const std::string& return_code)
{
	return ts3Functions.bandelall(id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_ban_list(const std::string& return_code)
{
	return ts3Functions.requestBanList(id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_complain_add(std::uint64_t target_client_db_id, const std::string& complain_reason, const std::string& return_code)
{
	return ts3Functions.requestComplainAdd(id, target_client_db_id, complain_reason.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_complain_del(std::uint64_t target_client_db_id, std::uint64_t from_client_db_id, const std::string& return_code)
{
	return ts3Functions.requestComplainDel(id, target_client_db_id, from_client_db_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_complain_del_all(std::uint64_t target_client_db_id, const std::string& return_code)
{
	return ts3Functions.requestComplainDelAll(id, target_client_db_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_complain_list(std::uint64_t target_client_db_id, const std::string& return_code)
{
	return ts3Functions.requestComplainList(id, target_client_db_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_group_list(const std::string& return_code)
{
	return ts3Functions.requestServerGroupList(id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_group_add(const std::string & group_name, int group_type, const std::string& return_code)
{
	return ts3Functions.requestServerGroupAdd(id, group_name.c_str(), group_type, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_group_del(std::uint64_t server_group_id, int force, const std::string& return_code)
{
	return ts3Functions.requestServerGroupDel(id, server_group_id, force, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_group_add_client(std::uint64_t server_group_id, std::uint64_t client_db_id, const std::string& return_code)
{
	return ts3Functions.requestServerGroupAddClient(id, server_group_id, client_db_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_group_del_client(std::uint64_t server_group_id, std::uint64_t client_db_id, const std::string& return_code)
{
	return ts3Functions.requestServerGroupDelClient(id, server_group_id, client_db_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_groups_by_client_id(std::uint64_t client_db_id, const std::string& return_code)
{
	return ts3Functions.requestServerGroupsByClientID(id, client_db_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_group_add_perm(std::uint64_t server_group_id, int continueonerror, const unsigned int* permission_ids, const int* permission_values, const int* permission_negateds, const int* permission_skips, int array_size, const std::string& return_code)
{
	return ts3Functions.requestServerGroupAddPerm(id, server_group_id, continueonerror, permission_ids, permission_values, permission_negateds, permission_skips, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_group_del_perm(std::uint64_t server_group_id, int continue_on_error, const unsigned int* permission_ids, int array_size, const std::string& return_code)
{
	return ts3Functions.requestServerGroupDelPerm(id, server_group_id, continue_on_error, permission_ids, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_group_perm_list(std::uint64_t server_group_id, const std::string& return_code)
{
	return ts3Functions.requestServerGroupPermList(id, server_group_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_group_client_list(std::uint64_t server_group_id, int with_names, const std::string& return_code)
{
	return ts3Functions.requestServerGroupClientList(id, server_group_id, with_names, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_group_list(const std::string& return_code)
{
	return ts3Functions.requestChannelGroupList(id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_group_add(const std::string& group_name, int group_type, const std::string& return_code)
{
	return ts3Functions.requestChannelGroupAdd(id, group_name.c_str(), group_type, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_group_del(std::uint64_t channel_group_id, int force, const std::string& return_code)
{
	return ts3Functions.requestChannelGroupDel(id, channel_group_id, force, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_group_add_perm(std::uint64_t channel_group_id, int continue_on_error, const unsigned int* permission_ids, const int* permission_values, int array_size, const std::string& return_code)
{
	return ts3Functions.requestChannelGroupAddPerm(id, channel_group_id, continue_on_error, permission_ids, permission_values, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_group_del_perm(std::uint64_t channel_group_id, int continue_on_error, const unsigned int* permission_ids, int array_size, const std::string& return_code)
{
	return ts3Functions.requestChannelGroupDelPerm(id, channel_group_id, continue_on_error, permission_ids, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_group_perm_list(std::uint64_t channel_group_id, const std::string& return_code)
{
	return ts3Functions.requestChannelGroupPermList(id, channel_group_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_set_client_channel_group(const std::uint64_t * channel_group_ids, const std::uint64_t * channel_ids, const std::uint64_t * client_db_ids, int array_size, const std::string& return_code)
{
	return ts3Functions.requestSetClientChannelGroup(id, channel_group_ids, channel_ids, client_db_ids, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_add_perm(std::uint64_t channel_id, const unsigned int* permission_ids, const int* permission_values, int array_size, const std::string& return_code)
{
	return ts3Functions.requestChannelAddPerm(id, channel_id, permission_ids, permission_values, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_del_perm(std::uint64_t channel_id, const unsigned int* permission_ids, int array_size, const std::string& return_code)
{
	return ts3Functions.requestChannelDelPerm(id, channel_id, permission_ids, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_perm_list(std::uint64_t channel_id, const std::string& return_code)
{
	return ts3Functions.requestChannelPermList(id, channel_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_client_add_perm(std::uint64_t client_db_id, const unsigned int* permission_ids, const int* permission_values, const int* permission_skips, int array_size, const std::string& return_code)
{
	return ts3Functions.requestClientAddPerm(id, client_db_id, permission_ids, permission_values, permission_skips, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_client_del_perm(std::uint64_t client_db_id, const unsigned int* permission_ids, int array_size, const std::string& return_code)
{
	return ts3Functions.requestClientDelPerm(id, client_db_id, permission_ids, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_client_perm_list(std::uint64_t client_db_id, const std::string& return_code)
{
	return ts3Functions.requestClientPermList(id, client_db_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_client_add_perm(std::uint64_t channel_id, std::uint64_t client_db_id, const unsigned int* permission_ids, const int* permission_values, int array_size, const std::string& return_code)
{
	return ts3Functions.requestChannelClientAddPerm(id, channel_id, client_db_id, permission_ids, permission_values, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_client_del_perm(std::uint64_t channel_id, std::uint64_t client_db_id, const unsigned int* permission_ids, int array_size, const std::string& return_code)
{
	return ts3Functions.requestChannelClientDelPerm(id, channel_id, client_db_id, permission_ids, array_size, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_client_perm_list(std::uint64_t channel_id, std::uint64_t client_db_id, const std::string& return_code)
{
	return ts3Functions.requestChannelClientPermList(id, channel_id, client_db_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::privilege_key_use(const std::string& token_key, const std::string& return_code)
{
	return ts3Functions.privilegeKeyUse(id, token_key.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_permission_list(const std::string& return_code)
{
	return ts3Functions.requestPermissionList(id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_permission_overview(std::uint64_t client_db_id, std::uint64_t channel_id, const std::string& return_code)
{
	return ts3Functions.requestPermissionOverview(id, client_db_id, channel_id, return_code.empty() ? nullptr : return_code.c_str());
}

void Connection_Base::print_message(const std::string & message, PluginMessageTarget message_target)
{
	ts3Functions.printMessage(id, message.c_str(), message_target);
}

void Connection_Base::send_plugin_command(const std::string& plugin_id, const std::string& command, Plugin_Target_Mode target_mode, const std::uint16_t* target_ids, const std::string& return_code)
{
	int plugin_target_mode = PluginTargetMode::PluginCommandTarget_MAX;
	switch (target_mode)
	{
	case Plugin_Target_Mode::PluginCommandTarget_CURRENT_CHANNEL:
		plugin_target_mode = PluginTargetMode::PluginCommandTarget_CURRENT_CHANNEL;
		break;
	case Plugin_Target_Mode::PluginCommandTarget_SERVER:
		plugin_target_mode = PluginTargetMode::PluginCommandTarget_SERVER;
		break;
	case Plugin_Target_Mode::PluginCommandTarget_CLIENT:
		plugin_target_mode = PluginTargetMode::PluginCommandTarget_CLIENT;
		break;
	case Plugin_Target_Mode::PluginCommandTarget_CURRENT_CHANNEL_SUBSCRIBED_CLIENTS:
		plugin_target_mode = PluginTargetMode::PluginCommandTarget_CURRENT_CHANNEL_SUBSCRIBED_CLIENTS;
		break;
	default:
		break;
	}
	ts3Functions.sendPluginCommand(id, plugin_id.c_str(), command.c_str(), plugin_target_mode, target_ids, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::get_server_connect_info(char* host, unsigned short* port, char* password, size_t max_len)
{
	return ts3Functions.getServerConnectInfo(id, host, port, password, max_len);
}

std::uint32_t Connection_Base::get_channel_connect_info(std::uint64_t channel_id, char* path, char* password, size_t max_len)
{
	return ts3Functions.getChannelConnectInfo(id, channel_id, path, password, max_len);
}

std::uint32_t Connection_Base::request_info_update(PluginItemType item_type, std::uint64_t item_id)
{
	return ts3Functions.requestInfoUpdate(id, item_type, item_id);
}

std::uint64_t Connection_Base::get_server_version()
{
	return ts3Functions.getServerVersion(id);
}

std::pair<std::uint32_t, bool> Connection_Base::is_whispering(std::uint16_t client_id)
{
	int32_t result;
	const auto error = ts3Functions.isWhispering(id, client_id, &result);
	return { error, result };
}

std::pair<std::uint32_t, bool> Connection_Base::is_receiving_whisper(std::uint16_t client_id)
{
	int32_t result;
	const auto error = ts3Functions.isReceivingWhisper(id, client_id, &result);
	return { error, result };
}

std::uint32_t Connection_Base::get_avatar(std::uint16_t client_id, char * result, size_t max_len)
{
	return ts3Functions.getAvatar(id, client_id, result, max_len);
}

std::pair<std::uint32_t, std::uint32_t> Connection_Base::get_permission_id_by_name(const std::string & permission_name)
{
	uint32_t result{ 0 };
	const auto error = ts3Functions.getPermissionIDByName(id, permission_name.c_str(), &result);
	return { error, result };
}

std::pair<std::uint32_t, int32_t> Connection_Base::get_client_needed_permission(const std::string & permission_name)
{
	return std::pair<std::uint32_t, int32_t>();
}

std::uint32_t Connection_Base::request_client_move(std::uint16_t client_id, std::uint64_t channel_id, const std::string& password, const std::string& return_code)
{
	return ts3Functions.requestClientMove(id, client_id, channel_id, password.c_str(), return_code.empty() ? nullptr : return_code.c_str()); // ToDo: utf8, return code
}

std::uint32_t Connection_Base::request_client_variables(std::uint16_t client_id, const std::string& return_code)
{
	return ts3Functions.requestClientVariables(id, client_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_client_kick_from_channel(std::uint16_t client_id, const std::string& kick_reason, const std::string& return_code)
{
	return ts3Functions.requestClientKickFromChannel(id, client_id, kick_reason.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_client_kick_from_server(std::uint16_t client_id, const std::string& kick_reason, const std::string& return_code)
{
	return ts3Functions.requestClientKickFromServer(id, client_id, kick_reason.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_delete(std::uint64_t channel_id, int force, const std::string& return_code)
{
	return ts3Functions.requestChannelDelete(id, channel_id, force, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_move(std::uint64_t channel_id, std::uint64_t new_channel_parent_id, std::uint64_t new_channel_order, const std::string& return_code)
{
	return ts3Functions.requestChannelMove(id, channel_id, new_channel_parent_id, new_channel_order, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_send_client_text_msg(const std::string& message, std::uint16_t client_id, const std::string& return_code)
{
	return ts3Functions.requestSendPrivateTextMsg(id, message.c_str(), client_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_send_channel_text_msg(const std::string& message, std::uint64_t channel_id, const std::string& return_code)
{
	return ts3Functions.requestSendChannelTextMsg(id, message.c_str(), channel_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_send_server_text_msg(const std::string& message, const std::string& return_code)
{
	return ts3Functions.requestSendServerTextMsg(id, message.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_connection_info(std::uint16_t client_id, const std::string& return_code)
{
	return ts3Functions.requestConnectionInfo(id, client_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_client_set_whisper_list(std::uint16_t client_id, std::vector<std::uint64_t>* target_channel_ids, std::vector<std::uint16_t>* target_client_ids, const std::string& return_code)
{
	if (target_channel_ids && target_channel_ids->size() != 0 && target_channel_ids->back() != 0)
		target_channel_ids->push_back(0);
	
	if (target_client_ids && target_client_ids->size() != 0 && target_client_ids->back() != 0)
		target_client_ids->push_back(0);

	return ts3Functions.requestClientSetWhisperList(id, client_id, target_channel_ids->size() != 0 ? target_channel_ids->data() : nullptr, target_client_ids->size() != 0 ? target_client_ids->data() : nullptr, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_change_channel_subscriptions(bool subscribe, std::vector<std::uint64_t>& channel_ids, const std::string& return_code)
{
	if (channel_ids.back() != 0)
		channel_ids.push_back(0);

	if (subscribe)
		return ts3Functions.requestChannelSubscribe(id, channel_ids.data(), return_code.empty() ? nullptr : return_code.c_str());

	return ts3Functions.requestChannelUnsubscribe(id, channel_ids.data(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_change_channel_subscriptions(bool subscribe, const std::string& return_code)
{
	if (subscribe)
		return ts3Functions.requestChannelSubscribeAll(id, return_code.empty() ? nullptr : return_code.c_str());

	return ts3Functions.requestChannelUnsubscribeAll(id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_channel_description(std::uint64_t channel_id, const std::string& return_code)
{
	return ts3Functions.requestChannelDescription(id, channel_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_clients_mute(bool mute, std::vector<std::uint16_t>& client_ids, const std::string& return_code)
{
	if (client_ids.back() != 0)
		client_ids.push_back(0);

	if (mute)
		return ts3Functions.requestMuteClients(id, client_ids.data(), return_code.empty() ? nullptr : return_code.c_str());
	
	return ts3Functions.requestUnmuteClients(id, client_ids.data(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_client_poke(std::uint16_t client_id, const std::string& message, const std::string& return_code)
{
	return ts3Functions.requestClientPoke(id, client_id, message.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_client_ids(const std::string& client_unique_id, const std::string& return_code)
{
	return ts3Functions.requestClientIDs(id, client_unique_id.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::client_chat_closed(const std::string& client_unique_id, std::uint16_t client_id, const std::string& return_code)
{
	return ts3Functions.clientChatClosed(id, client_unique_id.c_str(), client_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::client_chat_composing(std::uint16_t client_id, const std::string& return_code)
{
	return ts3Functions.clientChatComposing(id, client_id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_temporary_password_add(const std::string& password, const std::string& description, std::uint64_t duration, std::uint64_t target_channel_id, const std::string& target_channel_pw, const std::string& return_code)
{
	return ts3Functions.requestServerTemporaryPasswordAdd(id, password.c_str(), description.c_str(), duration, target_channel_id, target_channel_pw.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_temporary_password_del(const std::string& password, const std::string& return_code)
{
	return ts3Functions.requestServerTemporaryPasswordDel(id, password.c_str(), return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::request_server_temporary_password_list(const std::string& return_code)
{
	return ts3Functions.requestServerTemporaryPasswordList(id, return_code.empty() ? nullptr : return_code.c_str());
}

std::uint32_t Connection_Base::open_device(const std::string& backend, bool playback, const std::string& device)
{
	if (playback)
		return ts3Functions.openPlaybackDevice(id, backend.c_str(), device.c_str());

	return ts3Functions.openCaptureDevice(id, backend.c_str(), device.c_str());
}

std::tuple<std::uint32_t, std::string, bool> Connection_Base::get_current_device_name(bool playback)
{
	char* result = nullptr;
	int is_default = 0;
	std::uint32_t error = 0;
	if (playback)
		error = ts3Functions.getCurrentPlaybackDeviceName(id, &result, &is_default);
	else
		error = ts3Functions.getCurrentCaptureDeviceName(id, &result, &is_default);

	return { error, result, is_default };
}

std::pair<std::uint32_t, std::string> Connection_Base::get_current_backend(bool playback)
{
	char* result = nullptr;
	std::uint32_t error;
	if (playback)
		error = ts3Functions.getCurrentPlayBackMode(id, &result);
	else
		error = ts3Functions.getCurrentCaptureMode(id, &result);

	return { error, result };
}

std::uint32_t Connection_Base::initiate_graceful_playback_shutdown()
{
	return ts3Functions.initiateGracefulPlaybackShutdown(id);
}

std::uint32_t Connection_Base::close_device(bool playback)
{
	if (playback)
		return ts3Functions.closePlaybackDevice(id);

	return ts3Functions.closeCaptureDevice(id);
}

std::uint32_t Connection_Base::activate_capture_device()
{
	return ts3Functions.activateCaptureDevice(id);
}

std::pair<std::uint32_t, uint64_t> Connection_Base::play_wave_file_handle(const std::string & path, bool loop)
{
	uint64_t handle{ 0 };
	const auto error = ts3Functions.playWaveFileHandle(id, path.c_str(), loop ? 1 : 0, &handle);
	return { error, handle };
}

std::uint32_t Connection_Base::pause_wave_file_handle(std::uint64_t wave_handle, bool pause)
{
	return ts3Functions.pauseWaveFileHandle(id, wave_handle, pause ? 1 : 0);
}

std::uint32_t Connection_Base::close_wave_file_handle(std::uint64_t wave_handle)
{
	return ts3Functions.closeWaveFileHandle(id, wave_handle);
}

std::uint32_t Connection_Base::play_wave_file(const std::string & path)
{
	return ts3Functions.playWaveFile(id, path.c_str());
}

std::pair<std::uint32_t, float> Connection_Base::get_pre_processor_info(Pre_Processor_Info info)
{
	uint32_t error = ERROR_ok_no_update;
	float result;
	switch (info)
	{
	case Pre_Processor_Info::decibel_last_period:
		error = ts3Functions.getPreProcessorInfoValueFloat(id, "decibel_last_period", &result);
	default:
		return { ERROR_not_implemented, 0.0f };
	}

	return { error, result };
}

std::pair<std::uint32_t, std::string> Connection_Base::get_pre_processor_config_value(Pre_Processor_Config config)
{
	uint32_t error = ERROR_ok_no_update;
	char* result = nullptr;

	switch (config)
	{
	case Pre_Processor_Config::Name:
		error = ts3Functions.getPreProcessorConfigValue(id, "name", &result);
		break;
	case Pre_Processor_Config::Denoise:
		error = ts3Functions.getPreProcessorConfigValue(id, "denoise", &result);
		break;
	case Pre_Processor_Config::Voice_Activity_Detection:
		error = ts3Functions.getPreProcessorConfigValue(id, "vad", &result);
		break;
	case Pre_Processor_Config::Voice_Activity_Detection_Level:
		error = ts3Functions.getPreProcessorConfigValue(id, "voiceactivation_level", &result);
		break;
	case Pre_Processor_Config::Voice_Activity_Detection_Extrabuffersize:
		error = ts3Functions.getPreProcessorConfigValue(id, "vad_extrabuffersize", &result);
		break;
	case Pre_Processor_Config::Automatic_Gain_Correction:
		error = ts3Functions.getPreProcessorConfigValue(id, "agc", &result);
		break;
	case Pre_Processor_Config::Automatic_Gain_Correction_Level:
		error = ts3Functions.getPreProcessorConfigValue(id, "agc_level", &result);
		break;
	case Pre_Processor_Config::Automatic_Gain_Correction_Max_Gain:
		error = ts3Functions.getPreProcessorConfigValue(id, "agc_max_gain", &result);
		break;
	case Pre_Processor_Config::Echo_Cancellation:
		error = ts3Functions.getPreProcessorConfigValue(id, "echo_canceling", &result);
		break;
	default:
		return { ERROR_not_implemented, {} };
	}

	if (error != ERROR_ok)
		return { error, {} };

	if (!result)
		return { ERROR_undefined, {} };

	std::string ret{ result };
	ts3Functions.freeMemory(result);
	return { error, ret };
}

std::uint32_t Connection_Base::set_pre_processor_config_value(Pre_Processor_Config config, const std::string& value)
{
	switch (config)
	{
	case Pre_Processor_Config::Denoise:
		return ts3Functions.setPreProcessorConfigValue(id, "denoise" , value.c_str());
	case Pre_Processor_Config::Voice_Activity_Detection:
		return ts3Functions.setPreProcessorConfigValue(id, "vad" , value.c_str());
	case Pre_Processor_Config::Voice_Activity_Detection_Level:
		return ts3Functions.setPreProcessorConfigValue(id, "voiceactivation_level", value.c_str());
	case Pre_Processor_Config::Voice_Activity_Detection_Extrabuffersize:
		return ts3Functions.setPreProcessorConfigValue(id, "vad_extrabuffersize", value.c_str());
	case Pre_Processor_Config::Automatic_Gain_Correction:
		return ts3Functions.setPreProcessorConfigValue(id, "agc", value.c_str());
	case Pre_Processor_Config::Automatic_Gain_Correction_Level:
		return ts3Functions.setPreProcessorConfigValue(id, "agc_level", value.c_str());
	case Pre_Processor_Config::Automatic_Gain_Correction_Max_Gain:
		return ts3Functions.setPreProcessorConfigValue(id, "agc_max_gain", value.c_str());
	case Pre_Processor_Config::Echo_Cancellation:
		return ts3Functions.setPreProcessorConfigValue(id, "echo_canceling", value.c_str());
	default:
		break;
	}

	return ERROR_not_implemented;
}

std::pair<std::uint32_t, std::string> Connection_Base::get_encode_config(Encode_Config config)
{
	uint32_t error = ERROR_ok_no_update;
	char* result = nullptr;
	switch (config)
	{
	case Encode_Config::Name:
		error = ts3Functions.getEncodeConfigValue(id, "name", &result);
		break;
	case Encode_Config::Bitrate:
		error = ts3Functions.getEncodeConfigValue(id, "bitrate", &result);
		break;
	case Encode_Config::Quality:
		error = ts3Functions.getEncodeConfigValue(id, "quality", &result);
		break;
	default:
		return { ERROR_not_implemented, {} };
	};

	if (error != ERROR_ok)
		return { error, {} };

	if (!result)
		return { ERROR_undefined, {} };

	std::string ret{ result };
	ts3Functions.freeMemory(result);
	return { error, ret };
}

std::pair<std::uint32_t, float> Connection_Base::get_playback_config(Playback_Config config)
{
	uint32_t error = ERROR_ok_no_update;
	float result = 0.0f;
	switch (config)
	{
	case Playback_Config::Voice_Volume_db:
		error = ts3Functions.getPlaybackConfigValueAsFloat(id, "volume_modifier", &result);
		break;
	case Playback_Config::Wave_Volume_db:
		error = ts3Functions.getPlaybackConfigValueAsFloat(id, "volume_factor_wave", &result);
		break;
	default:
		return { ERROR_not_implemented, 0.0f };
	}
	return { error, result };
}

std::uint32_t Connection_Base::set_playback_config(Playback_Config config, float val)
{
	const auto config_val = std::to_string(val).c_str(); // ToDo: to_chars once compiler support is further ahead
	switch (config)
	{
		case Playback_Config::Voice_Volume_db:
			return ts3Functions.setPreProcessorConfigValue(id, "volume_modifier", config_val);
			break;
		case Playback_Config::Wave_Volume_db:
			return ts3Functions.setPreProcessorConfigValue(id, "volume_factor_wave", config_val);
			break;
		default:
			break;
	}
	return ERROR_not_implemented;
}

std::uint32_t Connection_Base::set_client_volume_modifier(std::uint16_t client_id, float value)
{
	return ts3Functions.setClientVolumeModifier(id, client_id, value);
}

std::uint32_t Connection_Base::set_voice_recording(bool start)
{
	if (start)
		return ts3Functions.startVoiceRecording(id);
	
	return ts3Functions.stopVoiceRecording(id);
}

std::uint32_t Connection_Base::system_set_3D_listener_attributes(const TS3_VECTOR* position, const TS3_VECTOR* forward, const TS3_VECTOR* up)
{
	return ts3Functions.systemset3DListenerAttributes(id, position, forward, up);
}

std::uint32_t Connection_Base::set_3D_wave_attributes(std::uint64_t wave_handle, const TS3_VECTOR* position)
{
	return ts3Functions.set3DWaveAttributes(id, wave_handle, position);
}

std::uint32_t Connection_Base::system_set_3D_settings(float distance_factor, float rolloff_scale)
{
	return ts3Functions.systemset3DSettings(id, distance_factor, rolloff_scale);
}

std::uint32_t Connection_Base::channel_set_3D_attributes(std::uint16_t client_id, const TS3_VECTOR* position)
{
	return ts3Functions.channelset3DAttributes(id, client_id, position);
}
