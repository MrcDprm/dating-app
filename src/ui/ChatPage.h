#pragma once

#include "core/Profile.h"

#include <QSet>
#include <QWidget>

class ChatProvider;
class Database;
class QLabel;
class QLineEdit;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

// Seçili eşleşmeyle sohbet ekranı. Karşı taraf yapay zekâ ile cevap verir.
class ChatPage : public QWidget
{
    Q_OBJECT

public:
    ChatPage(Database &db, ChatProvider &provider, QWidget *parent = nullptr);

    void setCurrentUser(const Profile &me);
    void openChat(const Profile &other);
    void setProvider(ChatProvider &provider);


signals:
    void unmatched(const QString &otherId);

private:
    void sendMessage();
    void requestReply();
    void onReplyReady(const QString &otherId, const QString &text);
    void onReplyFailed(const QString &otherId, const QString &message);
    void confirmUnmatch();
    void reloadMessages();
    void updateInputState();
    void addBubble(const QString &text, bool fromUser);

    Database &m_db;
    ChatProvider *m_provider = nullptr;
    Profile m_me;
    Profile m_other;
    QSet<QString> m_waitingFor; // cevabı beklenen sohbetler (karşı profil kimlikleri)

    QLabel *m_avatar = nullptr;
    QLabel *m_title = nullptr;
    QScrollArea *m_scroll = nullptr;
    QVBoxLayout *m_bubbles = nullptr;
    QLabel *m_typing = nullptr;
    QLabel *m_error = nullptr;
    QPushButton *m_retry = nullptr;
    QLineEdit *m_input = nullptr;
    QPushButton *m_send = nullptr;
};
