#pragma once

#include "core/definitions.h"
#include "core/talkers.h"
#include "core/translator.h"
#include "core/ts_context_menu_qt.h"
#include "core/ts_error.h"
#include "core/ts_infodata_qt.h"

#include <QtCore/QObject>

#include <gsl/pointers>
#include <gsl/span>

#include <string_view>

class Plugin_Base : public QObject
{
	Q_OBJECT

public:
	Plugin_Base(const char* plugin_id, QObject *parent = nullptr);

	const std::string& id() const;

	Translator& translator();
	TSContextMenu& context_menu();
	TSInfoData& info_data();
	Talkers& talkers();

	// Plugin funcs

	/* Required functions */
	//void setFunctionPointers(const struct TS3Functions funcs);
	int init();
	virtual int initialize() = 0;

	/*
	* Note:
	* If your plugin implements a settings dialog, it must be closed and deleted here, else the
	* TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
	*/
	virtual void shutdown() {};

	/* Optional functions */
	virtual void configure(void* handle, void* qParentWidget) {};
	/*void registerPluginID(const char* id);
	const char* commandKeyword();*/
	virtual int process_command(uint64 sch_id, const QString& command, QStringList& args) { return 1; };
	void currentServerConnectionChanged(uint64 serverConnectionHandlerID);
	virtual void on_current_server_connection_changed(uint64 sch_id) {};
	/*const char* infoTitle();
	void infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data);
	void freeMemory(void* data);
	int requestAutoload();
	void initMenus(struct PluginMenuItem*** menuItems, char** menuIcon);
	void initHotkeys(struct PluginHotkey*** hotkeys);*/

	/* Clientlib */
    void
    onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);
    virtual void on_connect_status_changed(uint64 sch_id, int new_status, unsigned int error_number){};
    /*void onNewChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 channelParentID);
    void onNewChannelCreatedEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 channelParentID,
    anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier); void
    onDelChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char*
    invokerName, const char* invokerUniqueIdentifier); void onChannelMoveEvent(uint64
    serverConnectionHandlerID, uint64 channelID, uint64 newChannelParentID, anyID invokerID, const char*
    invokerName, const char* invokerUniqueIdentifier); void onUpdateChannelEvent(uint64
    serverConnectionHandlerID, uint64 channelID); void onUpdateChannelEditedEvent(uint64
    serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char*
    invokerUniqueIdentifier);
    void onUpdateClientEvent(uint64 serverConnectionHandlerID, anyID clientID, anyID invokerID, const char*
    invokerName, const char* invokerUniqueIdentifier);*/
    void onClientMoveEvent(uint64 serverConnectionHandlerID,
                           anyID clientID,
                           uint64 oldChannelID,
                           uint64 newChannelID,
                           int visibility,
                           const char *moveMessage);
    virtual void on_client_move(uint64 sch_id,
                                anyID client_id,
                                uint64 old_channel_id,
                                uint64 new_channel_id,
                                int visibility,
                                anyID my_id,
                                const char *move_message){};
    // void onClientMoveSubscriptionEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64
    // oldChannelID, uint64 newChannelID, int visibility);
    void onClientMoveTimeoutEvent(uint64 serverConnectionHandlerID,
                                  anyID clientID,
                                  uint64 oldChannelID,
                                  uint64 newChannelID,
                                  int visibility,
                                  const char *timeoutMessage);
    virtual void on_client_move_timeout(
    uint64 sch_id, anyID client_id, uint64 old_channel_id, anyID my_id, const char *timeout_message){};
    void onClientMoveMovedEvent(uint64 serverConnectionHandlerID,
                                anyID clientID,
                                uint64 oldChannelID,
                                uint64 newChannelID,
                                int visibility,
                                anyID moverID,
                                const char *moverName,
                                const char *moverUniqueIdentifier,
                                const char *moveMessage);
    virtual void on_client_move_moved(uint64 sch_id,
                                      anyID client_id,
                                      uint64 old_channel_id,
                                      uint64 new_channel_id,
                                      int visibility,
                                      anyID my_id,
                                      anyID mover_id,
                                      const char *mover_name,
                                      const char *mover_unique_id,
                                      const char *move_message){};
    /*void onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID,
    uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char*
    kickerUniqueIdentifier, const char* kickMessage); void onClientKickFromServerEvent(uint64
    serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID
    kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage); void
    onClientIDsEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, anyID clientID,
    const char* clientName); void onClientIDsFinishedEvent(uint64 serverConnectionHandlerID); void
    onServerEditedEvent(uint64 serverConnectionHandlerID, anyID editerID, const char* editerName, const char*
    editerUniqueIdentifier); void onServerUpdatedEvent(uint64 serverConnectionHandlerID);*/

    int onServerError(uint64 sch_id,
                      const char *error_message,
                      unsigned int error,
                      const char *return_code,
                      const char *extra_message);

    virtual int on_server_error(com::teamspeak::connection_id_t connection_id,
                                std::string_view error_message,
                                ts_errc error,
                                std::string_view return_code,
                                std::string_view extra_message)
    {
        return 0;
    };

    /*void onServerStopEvent(uint64 serverConnectionHandlerID, const char* shutdownMessage);
    int  onTextMessageEvent(uint64 serverConnectionHandlerID, anyID targetMode, anyID toID, anyID fromID,
    const char* fromName, const char* fromUniqueIdentifier, const char* message, int ffIgnored);*/
    void onTalkStatusChangeEvent(uint64 serverConnectionHandlerID,
                                 int status,
                                 int isReceivedWhisper,
                                 anyID clientID);
    virtual void
    on_talk_status_changed(uint64 sch_id, int status, int is_received_whisper, anyID client_id, bool is_me){};
    /*void onConnectionInfoEvent(uint64 serverConnectionHandlerID, anyID clientID);
    void onServerConnectionInfoEvent(uint64 serverConnectionHandlerID);
    void onChannelSubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID);
    void onChannelSubscribeFinishedEvent(uint64 serverConnectionHandlerID);
    void onChannelUnsubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID);
    void onChannelUnsubscribeFinishedEvent(uint64 serverConnectionHandlerID);
    void onChannelDescriptionUpdateEvent(uint64 serverConnectionHandlerID, uint64 channelID);
    void onChannelPasswordChangedEvent(uint64 serverConnectionHandlerID, uint64 channelID);
    void onPlaybackShutdownCompleteEvent(uint64 serverConnectionHandlerID);
    void onSoundDeviceListChangedEvent(const char* modeID, int playOrCap);*/
    void onEditPlaybackVoiceDataEvent(
    uint64 serverConnectionHandlerID, anyID clientID, short *samples, int sampleCount, int channels);
    virtual void
    on_playback_pre_process(uint64 sch_id, anyID client_id, short *samples, int frame_count, int channels){};
    void onEditPostProcessVoiceDataEvent(uint64 serverConnectionHandlerID,
                                         anyID clientID,
                                         short *samples,
                                         int sampleCount,
                                         int channels,
                                         const unsigned int *channelSpeakerArray,
                                         unsigned int *channelFillMask);
    virtual void on_playback_post_process(uint64 sch_id,
                                          anyID client_id,
                                          gsl::span<int16_t> samples,
                                          std::int32_t channels,
                                          const std::uint32_t *channel_speaker_array,
                                          std::uint32_t *channel_fill_mask){};
    virtual void on_playback_master(uint64 sch_id,
                                    std::int16_t *samples,
                                    std::int32_t frame_count,
                                    std::int32_t channels,
                                    const std::uint32_t *channel_speaker_array,
                                    std::uint32_t *channel_fill_mask){};
    virtual void on_captured(uint64 sch_id,
                             std::int16_t *samples,
                             std::int32_t frame_count,
                             std::int32_t channels,
                             std::int32_t *edited){};
    virtual void
    on_custom_3d_rolloff_calculation(uint64 sch_id, anyID client_id, float distance, float *volume){};
    /*void onCustom3dRolloffCalculationWaveEvent(uint64 serverConnectionHandlerID, uint64 waveHandle, float
    distance, float* volume);
    void onUserLoggingMessageEvent(const char* logMessage, int logLevel, const char* logChannel, uint64 logID,
    const char* logTime, const char* completeLogString);*/

    /* Clientlib rare */
	/*void onClientBanFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, uint64 time, const char* kickMessage);
	int  onClientPokeEvent(uint64 serverConnectionHandlerID, anyID fromClientID, const char* pokerName, const char* pokerUniqueIdentity, const char* message, int ffIgnored);*/
	virtual void on_client_self_variable_update(uint64 sch_id, int flag, const char* old_value, const char* new_value) {};
	/*void onFileListEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* path, const char* name, uint64 size, uint64 datetime, int type, uint64 incompletesize, const char* returnCode);
	void onFileListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* path);
	void onFileInfoEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* name, uint64 size, uint64 datetime);*/
	virtual void on_server_group_list(uint64 sch_id, uint64 server_group_id, const char* name, int type, int icon_id, int save_db) {};
	virtual void on_server_group_list_finished(uint64 sch_id) {};
	/*void onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID);
	void onServerGroupPermListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip);
	void onServerGroupPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID);
	void onServerGroupClientListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, uint64 clientDatabaseID, const char* clientNameIdentifier, const char* clientUniqueID);*/
	virtual void on_channel_group_list(uint64 sch_id, uint64 channel_group_id, const char* name, int type, int icon_id, int save_db) {};
	virtual void on_channel_group_list_finished(uint64 sch_id) {};
	/*void onChannelGroupPermListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip);
	void onChannelGroupPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID);
	void onChannelPermListEvent(uint64 serverConnectionHandlerID, uint64 channelID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip);
	void onChannelPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID);
	void onClientPermListEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip);
	void onClientPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID);
	void onChannelClientPermListEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 clientDatabaseID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip);
	void onChannelClientPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 clientDatabaseID);
	void onClientChannelGroupChangedEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, uint64 channelID, anyID clientID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity);
	int  onServerPermissionErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, unsigned int failedPermissionID);
	void onPermissionListGroupEndIDEvent(uint64 serverConnectionHandlerID, unsigned int groupEndID);
	void onPermissionListEvent(uint64 serverConnectionHandlerID, unsigned int permissionID, const char* permissionName, const char* permissionDescription);
	void onPermissionListFinishedEvent(uint64 serverConnectionHandlerID);
	void onPermissionOverviewEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID, uint64 channelID, int overviewType, uint64 overviewID1, uint64 overviewID2, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip);
	void onPermissionOverviewFinishedEvent(uint64 serverConnectionHandlerID);
	void onServerGroupClientAddedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity);
	void onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity);
	void onClientNeededPermissionsEvent(uint64 serverConnectionHandlerID, unsigned int permissionID, int permissionValue);
	void onClientNeededPermissionsFinishedEvent(uint64 serverConnectionHandlerID);
	void onFileTransferStatusEvent(anyID transferID, unsigned int status, const char* statusMessage, uint64 remotefileSize, uint64 serverConnectionHandlerID);
	void onClientChatClosedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientUniqueIdentity);
	void onClientChatComposingEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientUniqueIdentity);
	void onServerLogEvent(uint64 serverConnectionHandlerID, const char* logMsg);
	void onServerLogFinishedEvent(uint64 serverConnectionHandlerID, uint64 lastPos, uint64 fileSize);
	void onMessageListEvent(uint64 serverConnectionHandlerID, uint64 messageID, const char* fromClientUniqueIdentity, const char* subject, uint64 timestamp, int flagRead);
	void onMessageGetEvent(uint64 serverConnectionHandlerID, uint64 messageID, const char* fromClientUniqueIdentity, const char* subject, const char* message, uint64 timestamp);
	void onClientDBIDfromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID);
	void onClientNamefromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName);
	void onClientNamefromDBIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName);
	void onComplainListEvent(uint64 serverConnectionHandlerID, uint64 targetClientDatabaseID, const char* targetClientNickName, uint64 fromClientDatabaseID, const char* fromClientNickName, const char* complainReason, uint64 timestamp);
	void onBanListEvent(uint64 serverConnectionHandlerID, uint64 banid, const char* ip, const char* name, const char* uid, uint64 creationTime, uint64 durationTime, const char* invokerName, uint64 invokercldbid, const char* invokeruid, const char* reason, int numberOfEnforcements, const char* lastNickName);
	void onClientServerQueryLoginPasswordEvent(uint64 serverConnectionHandlerID, const char* loginPassword);*/
	virtual void on_plugin_command(uint64 sch_id, const char* plugin_name, const char* plugin_command, anyID invoker_client_id, const char* invoker_name, const char* invoker_uid) {};
	/*void onIncomingClientQueryEvent(uint64 serverConnectionHandlerID, const char* commandText);
	void onServerTemporaryPasswordListEvent(uint64 serverConnectionHandlerID, const char* clientNickname, const char* uniqueClientIdentifier, const char* description, const char* password, uint64 timestampStart, uint64 timestampEnd, uint64 targetChannelID, const char* targetChannelPW);*/

	/* Client UI callbacks */
	//void onAvatarUpdated(uint64 serverConnectionHandlerID, anyID clientID, const char* avatarPath);
	void onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID);
    /*void onHotkeyEvent(const char* keyword);
    void onHotkeyRecordedEvent(const char* keyword, const char* key);
    void onClientDisplayNameChanged(uint64 serverConnectionHandlerID, anyID clientID, const char* displayName,
    const char* uniqueClientIdentifier); const char* keyDeviceName(const char* keyIdentifier); const char*
    displayKeyText(const char* keyIdentifier); const char* keyPrefix() {};*/

  private:
    const std::string kPluginId;

    gsl::owner<Translator *> m_translator = nullptr;
    gsl::owner<TSContextMenu *> m_context_menu = nullptr;
    gsl::owner<TSInfoData *> m_info_data = nullptr;
    gsl::owner<Talkers *> m_talkers = nullptr;

    anyID my_id_move_event(uint64 sch_id, anyID client_id, uint64 new_channel_id, int visibility);
};

