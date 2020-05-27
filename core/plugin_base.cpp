#include "core/plugin_base.h"

#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "ts3_functions.h"
#include "plugin.h"

#include "core/ts_logging_qt.h"
#include "core/ts_settings_qt.h"
#include "core/ts_helpers_qt.h"

Plugin_Base::Plugin_Base(const char* plugin_id, QObject *parent)
	: QObject(parent)
	, kPluginId(plugin_id)
{}

const std::string& Plugin_Base::id() const
{
	return kPluginId;
}

Translator& Plugin_Base::translator()
{
	if (!m_translator)
		m_translator = new Translator(this);

	return *m_translator;
}

TSContextMenu& Plugin_Base::context_menu()
{
	if (!m_context_menu)
		m_context_menu = new TSContextMenu(this);

	return *m_context_menu;
}

TSInfoData& Plugin_Base::info_data()
{
	if (!m_info_data)
		m_info_data = new TSInfoData(this);

	return *m_info_data;
}

Talkers& Plugin_Base::talkers()
{
	if (!m_talkers)
		m_talkers = new Talkers(this);

	return *m_talkers;
}

int Plugin_Base::init()
{
	TSLogging::Log("init");

#ifdef CONSOLE_OUTPUT
	freopen("CONOUT$", "wb", stdout);   //Makes printf work in Release mode (shouldn't been necessary, but is...)
#endif

	TSSettings::instance()->Init(TSHelpers::GetConfigPath());

	// load sqlite driver if necessary
	if (QSqlDatabase::connectionNames().isEmpty())
	{
		TSLogging::Log("Loading Database...");
		QSqlDatabase db;

		db = QSqlDatabase::addDatabase("QSQLITE");
		db.setDatabaseName(TSHelpers::GetConfigPath() + "settings.db");

		if (db.isValid())
			TSLogging::Log("Database is valid.");
		else
			TSLogging::Log("Database is not valid.");

		if (!db.open())
		{
			TSLogging::Error("Error loading settings.db; aborting init", 0, NULL);
			return 1;
		}
	}

	const auto kDerivedResult = initialize();

	// Support enabling the plugin while already connected
	uint64* servers;
	if (ts3Functions.getServerConnectionHandlerList(&servers) == ERROR_ok)
	{
		for (auto server = servers; *server != (uint64)NULL; ++server)
		{
			int status;
			if (ts3Functions.getConnectionStatus(*server, &status) != ERROR_ok)
				continue;

			if (status > STATUS_DISCONNECTED)
			{
				// Not used by now
				//uint64 channelID;
				for (int j = STATUS_CONNECTING; j <= status; ++j)
				{
					ts3plugin_onConnectStatusChangeEvent(*server, j, ERROR_ok);
					/*if (j == STATUS_CONNECTED)
					{
					// Get our channel and publish it as default channel
					anyID myID;
					if(ts3Functions.getClientID(*server,&myID) == ERROR_ok)
					{
					if(ts3Functions.getChannelOfClient(*server,myID,&channelID) == ERROR_ok)
					{
					uint64 channelParentID;
					if (ts3Functions.getParentChannelOfChannel(*server,channelID,&channelParentID) == ERROR_ok)
					ts3plugin_onNewChannelEvent(*server, channelID, channelParentID);
					}
					}
					}
					else if (j == STATUS_CONNECTION_ESTABLISHING)
					{
					// publish all other channels
					uint64* channelList;
					if (ts3Functions.getChannelList(*server,&channelList) == ERROR_ok)
					{
					for (int k=0;channelList[k] != NULL;++k)
					{
					if (channelList[k] == channelID)
					continue;

					uint64 channelParentID;
					if (ts3Functions.getParentChannelOfChannel(*server,channelList[k],&channelParentID) == ERROR_ok)
					ts3plugin_onNewChannelEvent(*server, channelList[k], channelParentID); // ToDo: Hope it's not necessary to sort those
					}
					ts3Functions.freeMemory(channelList);
					}
					}*/
				}
				if (status == STATUS_CONNECTION_ESTABLISHED)
				{
					ts3plugin_currentServerConnectionChanged(*server);
					unsigned int error;
					if ((error = talkers().RefreshTalkers(*server)) != ERROR_ok)
						TSLogging::Error("Error refreshing talkers: ", *server, error);
				}
			}
		}
		ts3Functions.freeMemory(servers);

		// Get the active server tab
		auto scHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
		if (scHandlerID != 0)
			ts3plugin_currentServerConnectionChanged(scHandlerID);
	}

	TSLogging::Log("init done");
	return kDerivedResult;	/* 0 = success, 1 = failure */
}

void Plugin_Base::currentServerConnectionChanged(uint64 serverConnectionHandlerID)
{
	// event will fire twice on connecting to a new tab; first before connecting, second after established by our manual trigger
	unsigned int error;
	int status;
	if ((error = ts3Functions.getConnectionStatus(serverConnectionHandlerID, &status)) != ERROR_ok)
	{
		TSLogging::Error("ts3plugin_currentServerConnectionChanged", serverConnectionHandlerID, error);
		return;
	}
	if (status != STATUS_CONNECTION_ESTABLISHED)
		return;

	on_current_server_connection_changed(serverConnectionHandlerID);
	//    infoData->setHomeId(serverConnectionHandlerID);
}

void Plugin_Base::onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber)
{
	talkers().onConnectStatusChangeEvent(serverConnectionHandlerID, newStatus, errorNumber);
	if (newStatus == STATUS_CONNECTION_ESTABLISHED)
	{
		currentServerConnectionChanged(serverConnectionHandlerID);
		// Fake client move of myself

		unsigned int error;
		// Get My Id on this handler
		anyID myID;
		if ((error = ts3Functions.getClientID(serverConnectionHandlerID, &myID)) != ERROR_ok)
			TSLogging::Error("(ts3plugin_onConnectStatusChangeEvent) Error getting my clientID");
		{
			// Get My channel on this handler
			uint64 channelID;
			if ((error = ts3Functions.getChannelOfClient(serverConnectionHandlerID, myID, &channelID)) != ERROR_ok)
				TSLogging::Error("(ts3plugin_onConnectStatusChangeEvent) Error getting my clients channel id", serverConnectionHandlerID, error);
			else
				onClientMoveEvent(serverConnectionHandlerID, myID, 0, channelID, ENTER_VISIBILITY, "");
		}
	}
	on_connect_status_changed(serverConnectionHandlerID, newStatus, errorNumber);
}

void Plugin_Base::onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char * moveMessage)
{
	const auto kMyId = my_id_move_event(serverConnectionHandlerID, clientID, newChannelID, visibility);
	if (kMyId)
		on_client_move(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, kMyId, moveMessage);
}

void Plugin_Base::onClientMoveTimeoutEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char * timeoutMessage)
{
	const auto kMyId = my_id_move_event(serverConnectionHandlerID, clientID, newChannelID, visibility);
	if (kMyId)
		on_client_move_timeout(serverConnectionHandlerID, clientID, newChannelID, kMyId, timeoutMessage);
}

void Plugin_Base::onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char * moverName, const char * moverUniqueIdentifier, const char * moveMessage)
{
	const auto kMyId = my_id_move_event(serverConnectionHandlerID, clientID, newChannelID, visibility);
	if (kMyId)
		on_client_move_moved(serverConnectionHandlerID, clientID, oldChannelID, newChannelID, visibility, kMyId, moverID, moverName, moverUniqueIdentifier, moveMessage);
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
	on_playback_post_process(serverConnectionHandlerID, clientID, samples, sampleCount, channels, channelSpeakerArray, channelFillMask);
}

void Plugin_Base::onMenuItemEvent(uint64 serverConnectionHandlerID, PluginMenuType type, int menuItemID, uint64 selectedItemID)
{
	context_menu().onMenuItemEvent(serverConnectionHandlerID, type, menuItemID, selectedItemID);
	PluginItemType itype;
	switch (type)
	{
	case PLUGIN_MENU_TYPE_GLOBAL:
		/* Global menu item was triggered. selectedItemID is unused and set to zero. */
		itype = PLUGIN_SERVER;   //admittedly not the same, however...
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
		itype = PLUGIN_SERVER;
		break;
	}
	info_data().RequestUpdate(serverConnectionHandlerID, selectedItemID, itype);
}

anyID Plugin_Base::my_id_move_event(uint64 sch_id, anyID client_id, uint64 new_channel_id, int visibility)
{
	unsigned int error;
	if (new_channel_id == 0)  // When we disconnect, we get moved to chan 0 before the connection event
	{                       // However, we aren't able to get our own id etc. anymore via the API for comparison
		int con_status;     // Therefor, unless we cache our ids for no other benefit, this is a workaround by filtering those out
		if ((error = ts3Functions.getConnectionStatus(sch_id, &con_status)) != ERROR_ok)
		{
			TSLogging::Error("(filter_move_event)", sch_id, error);
			return 0;
		}
		if (con_status == STATUS_DISCONNECTED)
			return 0;
	}
	else if ((visibility != LEAVE_VISIBILITY) && (TSHelpers::IsClientQuery(sch_id, client_id)))
		return 0;

	// Get My Id on this handler
	anyID my_id;
	if ((error = ts3Functions.getClientID(sch_id, &my_id)) != ERROR_ok)
	{
		TSLogging::Error("(ts3plugin_onClientMoveEvent)", sch_id, error);
		return 0;
	}
	return my_id;
}
