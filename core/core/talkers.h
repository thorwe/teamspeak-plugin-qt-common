#pragma once

#include "core/definitions.h"

#include "teamspeak/public_definitions.h"

#include <QtCore/QObject>

#include <shared_mutex>

class TalkInterface
{
  public:
    virtual bool onTalkStatusChanged(com::teamspeak::connection_id_t connection_id,
                                     int status,
                                     bool is_received_whisper,
                                     com::teamspeak::client_id_t client_id,
                                     bool is_me) = 0;
};
Q_DECLARE_INTERFACE(TalkInterface, "com.teamspeak.TalkInterface/1.0")

class Talkers : public QObject
{
    Q_OBJECT

  public:
    Talkers(QObject *parent = nullptr);

    struct Talkers_Info
    {
        com::teamspeak::connection_id_t connection_id{0};
        com::teamspeak::client_id_t client_id{0};
        bool is_whispering{false};
    };

    bool onTalkStatusChangeEvent(com::teamspeak::connection_id_t connection_id,
                                 int status,
                                 int is_received_whisper,
                                 com::teamspeak::client_id_t client_id);
    void onConnectStatusChangeEvent(com::teamspeak::connection_id_t connection_id,
                                    int new_status,
                                    unsigned int error_number);

    com::teamspeak::connection_id_t isMeTalking() const;

    // Unlike with cached information, we won't get talkers out of view
    std::error_code RefreshTalkers(com::teamspeak::connection_id_t connection_id);

    // Unlike with cached information, we won't get talkers out of view
    std::error_code RefreshAllTalkers();

    void DumpTalkStatusChanges(QObject *p, int status);

    enum class Talker_Type
    {
        All = 0,
        Talkers,
        Whisperers
    };
    std::vector<Talkers_Info> get_infos(Talker_Type talker_type = Talker_Type::All,
                                        com::teamspeak::connection_id_t connection_id = 0) const;

  private:
    com::teamspeak::connection_id_t m_me_talking_connection_id = 0;
    bool m_me_talking_is_whisper{false};

    std::vector<Talkers_Info> m_talkers;

    // protects the m_talkers and m_me... states)
    std::shared_mutex m_mutex;
};
