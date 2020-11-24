#include "core/plugin_base.h"

#include "plugin.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"

#include "core/ts_functions.h"
#include "core/ts_helpers_qt.h"
#include "core/ts_logging_qt.h"
#include "core/ts_settings_qt.h"

#include "gsl/gsl_util"

using namespace com::teamspeak::pluginsdk;

Plugin_Base::Plugin_Base(const char* plugin_id, QObject *parent)
	: QObject(parent)
	, kPluginId(plugin_id)
{}

auto Plugin_Base::id() const -> const std::string &
{
	return kPluginId;
}
auto Plugin_Base::translator() -> Translator &
{
	if (!m_translator)
		m_translator = new Translator(this);

	return *m_translator;
}

auto Plugin_Base::context_menu() -> TSContextMenu &
{
	if (!m_context_menu)
		m_context_menu = new TSContextMenu(this);

	return *m_context_menu;
}

auto Plugin_Base::info_data() -> TSInfoData &
{
	if (!m_info_data)
		m_info_data = new TSInfoData(this);

	return *m_info_data;
}

auto Plugin_Base::talkers() -> Talkers &
{
	if (!m_talkers)
		m_talkers = new Talkers(this);

	return *m_talkers;
}

auto Plugin_Base::init() -> int
{
	TSLogging::Log("init");

#ifdef CONSOLE_OUTPUT
	freopen("CONOUT$", "wb", stdout);   //Makes printf work in Release mode (shouldn't been necessary, but is...)
#endif

    TSSettings::instance()->Init(TSHelpers::GetPath(teamspeak::plugin::Path::Config));

	// load sqlite driver if necessary
	if (QSqlDatabase::connectionNames().isEmpty())
	{
		TSLogging::Log("Loading Database...");
		QSqlDatabase db;

		db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(TSHelpers::GetPath(teamspeak::plugin::Path::Config) + "settings.db");

		if (db.isValid())
            TSLogging::Log("Database is valid.");
        else
            TSLogging::Log("Database is not valid.");

        if (!db.open())
        {
            TSLogging::Error("Error loading settings.db; aborting init");
            return 1;
        }
    }

    const auto kDerivedResult = initialize();

    // Support enabling the plugin while already connected
    if (auto [error_get_connection_ids, connection_ids] = funcs::get_server_connection_handler_ids();
        error_get_connection_ids == ts_errc::ok)
    {
        for (const auto connection_id : connection_ids)
        {
            if (auto [error_connection_status, connection_status] =
                funcs::get_connection_status(connection_id);
                error_connection_status == ts_errc::ok)
            {
                if (connection_status > STATUS_DISCONNECTED)
                {
                    // Not used by now
                    //uint64 channelID;
                    for (int j = STATUS_CONNECTING; j <= connection_status; ++j)
                    {
                        ts3plugin_onConnectStatusChangeEvent(connection_id, j, 0);
                        /*if (j == STATUS_CONNECTED)
                        {
                        // Get our channel and publish it as default channel
                        anyID myID;
                        if(ts_funcs.getClientID(*server,&myID) == ts_errc::ok)
                        {
                        if(ts_funcs.getChannelOfClient(*server,myID,&channelID) == ts_errc::ok)
                        {
                        uint64 channelParentID;
                        if (ts_funcs.getParentChannelOfChannel(*server,channelID,&channelParentID) ==
                        ts_errc::ok) ts3plugin_onNewChannelEvent(*server, channelID, channelParentID);
                        }
                        }
                        }
                        else if (j == STATUS_CONNECTION_ESTABLISHING)
                        {
                        // publish all other channels
                        uint64* channelList;
                        if (ts_funcs.getChannelList(*server,&channelList) == ts_errc::ok)
                        {
                        for (int k=0;channelList[k] != NULL;++k)
                        {
                        if (channelList[k] == channelID)
                        continue;

                        uint64 channelParentID;
                        if (ts_funcs.getParentChannelOfChannel(*server,channelList[k],&channelParentID) ==
                        ts_errc::ok) ts3plugin_onNewChannelEvent(*server, channelList[k], channelParentID); //
                        ToDo: Hope it's not necessary to sort those
                        }
                        ts_funcs.freeMemory(channelList);
                        }
                        }*/
                    }
                    if (connection_status == STATUS_CONNECTION_ESTABLISHED)
                    {
                        ts3plugin_currentServerConnectionChanged(connection_id);
                        if (const auto error_refresh = talkers().RefreshTalkers(connection_id);
                            error_refresh != ts_errc::ok)
                            TSLogging::Error("Error refreshing talkers: ", connection_id, error_refresh);
                    }
                }
            }

            if (const auto current_connection_id = funcs::get_current_server_connection_handler_id(); current_connection_id != 0)
                ts3plugin_currentServerConnectionChanged(current_connection_id);
        }
    }

	TSLogging::Log("init done");
	return kDerivedResult;	/* 0 = success, 1 = failure */
}

void Plugin_Base::currentServerConnectionChanged(uint64 connection_id)
{
	// event will fire twice on connecting to a new tab; first before connecting, second after established by our manual trigger
    auto[error_connection_status, connection_status] = funcs::get_connection_status(connection_id);
    if (ts_errc::ok != error_connection_status)
    {
        TSLogging::Error("ts3plugin_currentServerConnectionChanged", connection_id, error_connection_status);
        return;
    }

    if (connection_status != STATUS_CONNECTION_ESTABLISHED)
		return;

    on_current_server_connection_changed(connection_id);
}

void Plugin_Base::onConnectStatusChangeEvent(uint64 connection_id, int newStatus, unsigned int errorNumber)
{
    talkers().onConnectStatusChangeEvent(connection_id, newStatus, errorNumber);
	if (newStatus == STATUS_CONNECTION_ESTABLISHED)
	{
        currentServerConnectionChanged(connection_id);
		// Fake client move of myself

		// Get My Id on this handler
        const auto [error_my_id, my_id] = funcs::get_client_id(connection_id);
        if (ts_errc::ok != error_my_id)
        {
            TSLogging::Error("(onConnectStatusChangeEvent) Error getting my clientID");
        }
        else
        {
            const auto [error_channel, channel_id] = funcs::get_channel_of_client(connection_id, my_id);
            if (ts_errc::ok != error_channel)
            {
                TSLogging::Error("(onConnectStatusChangeEvent) Error getting my clients channel id", connection_id, error_channel);
            }
            else
            {
                onClientMoveEvent(connection_id, my_id, 0, channel_id, ENTER_VISIBILITY, "");
            }
        }
	}
    on_connect_status_changed(connection_id, newStatus, errorNumber);
}

void Plugin_Base::onClientMoveEvent(uint64 connection_id, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char * moveMessage)
{
    const auto kMyId = my_id_move_event(connection_id, clientID, newChannelID, visibility);
	if (kMyId)
        on_client_move(connection_id, clientID, oldChannelID, newChannelID, visibility, kMyId, moveMessage);
}

void Plugin_Base::onClientMoveTimeoutEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char * timeoutMessage)
{
	const auto kMyId = my_id_move_event(serverConnectionHandlerID, clientID, newChannelID, visibility);
	if (kMyId)
        on_client_move_timeout(serverConnectionHandlerID, clientID, oldChannelID, kMyId, timeoutMessage);
}

void Plugin_Base::onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char * moverName, const char * moverUniqueIdentifier, const char * moveMessage)
{
	const auto kMyId = my_id_move_event(serverConnectionHandlerID, clientID, newChannelID, visibility);
	if (kMyId)
		on_client_move_moved(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, kMyId, moverID, moverName, moverUniqueIdentifier, moveMessage);
}

auto Plugin_Base::onServerError(uint64 sch_id,
                                const char *error_message,
                                unsigned int error,
                                const char *return_code,
                                const char *extra_message) -> int
{
    return on_server_error(sch_id, error_message ? error_message : std::string_view{},
                           com::teamspeak::to_ts_errc(error), return_code ? return_code : std::string_view{},
                           extra_message ? extra_message : std::string_view{});
}

void Plugin_Base::onTalkStatusChangeEvent(uint64 serverConnectionHandlerID, int status, int isReceivedWhisper, anyID clientID)
{
	const auto kIsMe = talkers().onTalkStatusChangeEvent(serverConnectionHandlerID, status, isReceivedWhisper, clientID);
	on_talk_status_changed(serverConnectionHandlerID, status, isReceivedWhisper, clientID, kIsMe);
}

void Plugin_Base::onEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short * samples, int sampleCount, int channels)
{
	on_playback_pre_process(serverConnectionHandlerID, clientID, samples, sampleCount, channels);
}

void Plugin_Base::onEditPostProcessVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask)
{
    on_playback_post_process(serverConnectionHandlerID, clientID,
                             {samples, gsl::narrow_cast<size_t>(sampleCount * channels)}, channels,
                             channelSpeakerArray, channelFillMask);
}

void Plugin_Base::onMenuItemEvent(uint64 serverConnectionHandlerID, PluginMenuType type, int menuItemID, uint64 selectedItemID)
{
	context_menu().onMenuItemEvent(serverConnectionHandlerID, type, menuItemID, selectedItemID);
    PluginItemType itype = PLUGIN_SERVER;
    switch (type)
	{
	case PLUGIN_MENU_TYPE_GLOBAL:
		/* Global menu item was triggered. selectedItemID is unused and set to zero. */
        // itype = PLUGIN_SERVER;   //admittedly not the same, however...
        break;
	case PLUGIN_MENU_TYPE_CHANNEL:
		/* Channel contextmenu item was triggered. selectedItemID is the channelID of the selected channel */
		itype = PLUGIN_CHANNEL;
		break;
	case PLUGIN_MENU_TYPE_CLIENT:
		/* Client contextmenu item was triggered. selectedItemID is the clientID of the selected client */
		itype = PLUGIN_CLIENT;
		break;
	default:
		break;
	}
	info_data().RequestUpdate(serverConnectionHandlerID, selectedItemID, itype);
}

auto Plugin_Base::my_id_move_event(uint64 sch_id, anyID client_id, uint64 new_channel_id, int visibility)
-> anyID
{
    if (new_channel_id == 0)
    {
        // When we disconnect, we get moved to chan 0 before the connection event
        // However, we aren't able to get our own id etc. anymore via the API for comparison
        // Therefor, unless we cache our ids for no other benefit, this is a workaround by filtering those out
        const auto[error_connection_status, connection_status] = funcs::get_connection_status(sch_id);
        if (ts_errc::ok != error_connection_status)
        {
            TSLogging::Error("(filter_move_event)", sch_id, error_connection_status);
            return 0;
        }
        if (STATUS_DISCONNECTED == connection_status)
			return 0;
	}
    else if ((LEAVE_VISIBILITY != visibility) && (TSHelpers::is_query_client(sch_id, client_id)))
        return 0;

    const auto [error_my_id, my_id] = funcs::get_client_id(sch_id);
    if (ts_errc::ok != error_my_id)
    {
        TSLogging::Error("(ts3plugin_onClientMoveEvent)", sch_id, error_my_id);
        return 0;
    }
	return my_id;
}
