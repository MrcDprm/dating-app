#pragma once

#include "core/Profile.h"

#include <QDateTime>
#include <QList>
#include <QString>

#include <optional>

struct UserAccount
{
    QString username;
    QString passwordHash;
    QString profileId;
};

struct ChatMessage
{
    bool fromUser = true; // false: karşı taraf (yapay zekâ) yazdı
    QString text;
    QDateTime sentAt;
};

class Database
{
public:
    Database();
    ~Database();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    // Veritabanını açar, tablolar yoksa oluşturur. path ":memory:" olursa bellekte çalışır (testler için)
    bool open(const QString &path);
    QString lastError() const;

    // Profiller
    bool saveProfile(const Profile &profile);
    std::optional<Profile> profile(const QString &id) const;
    QList<Profile> unseenProfiles(const QString &viewerId) const;
    bool hasAiProfiles() const;

    // Hesaplar
    bool createAccount(const QString &username, const QString &passwordHash, const Profile &profile);
    std::optional<UserAccount> account(const QString &username) const;

    // Beğen / geç ve eşleşmeler
    bool recordSwipe(const QString &fromId, const QString &toId, bool liked);
    bool isMatch(const QString &userId, const QString &otherId) const;
    QList<Profile> matches(const QString &userId) const;
    bool unmatch(const QString &userId, const QString &otherId);
    bool resetPasses(const QString &userId);


    // Mesajlar
    bool addMessage(const QString &userId, const QString &otherId, bool fromUser, const QString &text);
    QList<ChatMessage> messages(const QString &userId, const QString &otherId) const;

private:
    bool createTables();
    QList<Profile> queryProfiles(const QString &sql, const QString &bindId) const;

    QString m_connectionName;
    mutable QString m_lastError;
};
