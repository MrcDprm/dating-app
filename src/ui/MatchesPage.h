#pragma once

#include "core/Profile.h"

#include <QWidget>

class ChatPage;
class ChatProvider;
class Database;
class QLabel;
class QListWidget;
class QStackedWidget;

// Eşleşmeler sekmesi: solda eşleşme listesi, sağda seçili kişiyle sohbet
class MatchesPage : public QWidget
{
    Q_OBJECT

public:
    MatchesPage(Database &db, ChatProvider &provider, QWidget *parent = nullptr);

    void setCurrentUser(const Profile &me);
    void refresh();
    void openChat(const QString &otherId);
    void setProvider(ChatProvider &provider);


private:
    void onSelectionChanged();

    Database &m_db;
    Profile m_me;
    QList<Profile> m_matches;

    QListWidget *m_list = nullptr;
    QStackedWidget *m_stack = nullptr;
    QLabel *m_placeholder = nullptr;
    ChatPage *m_chat = nullptr;
};
