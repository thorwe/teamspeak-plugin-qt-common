#pragma once

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QMutex>

#include "teamspeak/public_definitions.h"
#include "plugin_definitions.h"

class Plugin_Base;

class ContextMenuInterface
{
public:
    virtual void onContextMenuEvent(uint64 serverConnectionHandlerID, PluginMenuType type, int menuItemID, uint64 selectedItemID) = 0;
};
Q_DECLARE_INTERFACE(ContextMenuInterface, "com.teamspeak.ContextMenuInterface/1.0")

class TSContextMenu : public QObject
{
    Q_OBJECT

public:
	TSContextMenu(Plugin_Base* plugin);

    bool setMainIcon(QString icon);
    int Register(QObject *p, PluginMenuType type, QString text, QString icon);

    void onInitMenus(struct PluginMenuItem ***menuItems, char **menuIcon);
    void onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID);

signals:
    void MenusInitialized();
    void FireContextMenuEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID);

private:
    QList<PluginMenuItem*> m_Items;
    QVector<QPointer<QObject> > m_Callbacks;
    QString m_MainIcon;
    bool m_isInit = false;
};
