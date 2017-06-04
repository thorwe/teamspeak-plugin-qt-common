#pragma once

#include <QtCore/QObject>
#include <QtCore/QHash>
#include "teamspeak/public_definitions.h"
#include "dsp_volume.h"

class Volumes : public QObject
{
    Q_OBJECT

public:

    enum class Volume_Type : uint_least8_t
    {
        MANUAL = 0,
        DUCKER,
        AGMU
    };

    explicit Volumes(QObject *parent = 0, Volume_Type volume_type = Volume_Type::MANUAL);

    DspVolume* AddVolume(uint64 serverConnectionHandlerID, anyID clientID);
    void DeleteVolume(DspVolume *dspObj);
    void RemoveVolume(uint64 serverConnectionHandlerID, anyID clientID);
    void RemoveVolumes(uint64 serverConnectionHandlerID);
    void RemoveVolumes();
    bool ContainsVolume(uint64 serverConnectionHandlerID, anyID clientID);
    DspVolume* GetVolume(uint64 serverConnectionHandlerID, anyID clientID);

public slots:
    void onConnectStatusChanged(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);

private:
    QHash<QPair<uint64,anyID>, DspVolume* > m_volumes;
    Volume_Type m_volume_type;
};
