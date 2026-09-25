#include "Database.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

#include <algorithm>

namespace {

Gender genderFromInt(int value)
{
    switch (value) {
    case int(Gender::Man):
        return Gender::Man;
    case int(Gender::Woman):
        return Gender::Woman;
    default:
        return Gender::Other;
    }
}

Seeking seekingFromInt(int value)
{
    switch (value) {
    case int(Seeking::Men):
        return Seeking::Men;
    case int(Seeking::Women):
        return Seeking::Women;
    default:
        return Seeking::Everyone;
    }
}

QString interestsToJson(const QStringList &interests)
{
    return QString::fromUtf8(QJsonDocument(QJsonArray::fromStringList(interests)).toJson(QJsonDocument::Compact));
}

QStringList interestsFromJson(const QString &json)
{
    QStringList result;
    const QJsonArray array = QJsonDocument::fromJson(json.toUtf8()).array();
    for (const QJsonValue &value : array) {
        if (value.isString() && interestCatalog().contains(value.toString()))
            result.append(value.toString());
    }
    return result;
}

// Sorgunun o anki satırını Profile nesnesine çevirir; bozuk değerler güvenli aralığa çekilir
Profile profileFromQuery(const QSqlQuery &query)
{
    Profile profile;
    profile.id = query.value("id").toString();
    profile.name = query.value("name").toString();
    profile.age = std::clamp(query.value("age").toInt(), kMinAge, kMaxAge);
    profile.gender = genderFromInt(query.value("gender").toInt());
    profile.seeking = seekingFromInt(query.value("seeking").toInt());
    profile.minAge = std::clamp(query.value("min_age").toInt(), kMinAge, kMaxAge);
    profile.maxAge = std::clamp(query.value("max_age").toInt(), kMinAge, kMaxAge);
    profile.city = query.value("city").toString();
    profile.interests = interestsFromJson(query.value("interests").toString());
    profile.bio = query.value("bio").toString();
    profile.photoPath = query.value("photo_path").toString();
    profile.isAiPersona = query.value("is_ai").toBool();
    profile.personaStyle = query.value("persona_style").toString();
    return profile;
}

} // namespace

Database::Database()
    : m_connectionName(QUuid::createUuid().toString())
{
}

Database::~Database()
{
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool Database::open(const QString &path)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(path);
    if (!db.open()) {
        m_lastError = db.lastError().text();
        return false;
    }

    QSqlQuery pragma(db);
    pragma.exec("PRAGMA foreign_keys = ON");
    return createTables();
}

QString Database::lastError() const
{
    return m_lastError;
}

bool Database::createTables()
{
    const QStringList statements = {
        R"(CREATE TABLE IF NOT EXISTS profiles (
            id            TEXT PRIMARY KEY,
            name          TEXT NOT NULL,
            age           INTEGER NOT NULL,
            gender        INTEGER NOT NULL,
            seeking       INTEGER NOT NULL,
            min_age       INTEGER NOT NULL,
            max_age       INTEGER NOT NULL,
            city          TEXT,
            interests     TEXT NOT NULL DEFAULT '[]',
            bio           TEXT,
            photo_path    TEXT,
            is_ai         INTEGER NOT NULL DEFAULT 0,
            persona_style TEXT
        ))",
        R"(CREATE TABLE IF NOT EXISTS accounts (
            username      TEXT PRIMARY KEY COLLATE NOCASE,
            password_hash TEXT NOT NULL,
            profile_id    TEXT NOT NULL UNIQUE REFERENCES profiles(id) ON DELETE CASCADE
        ))",
        R"(CREATE TABLE IF NOT EXISTS swipes (
            from_id    TEXT NOT NULL REFERENCES profiles(id) ON DELETE CASCADE,
            to_id      TEXT NOT NULL REFERENCES profiles(id) ON DELETE CASCADE,
            liked      INTEGER NOT NULL,
            PRIMARY KEY (from_id, to_id)
        ))",
        R"(CREATE TABLE IF NOT EXISTS messages (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id    TEXT NOT NULL REFERENCES profiles(id) ON DELETE CASCADE,
            other_id   TEXT NOT NULL REFERENCES profiles(id) ON DELETE CASCADE,
            from_user  INTEGER NOT NULL,
            text       TEXT NOT NULL,
            sent_at    TEXT NOT NULL
        ))",
        "CREATE INDEX IF NOT EXISTS idx_messages_pair ON messages(user_id, other_id)",
    };

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    for (const QString &sql : statements) {
        if (!query.exec(sql)) {
            m_lastError = query.lastError().text();
            return false;
        }
    }
    return true;
}

bool Database::saveProfile(const Profile &profile)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        INSERT INTO profiles (id, name, age, gender, seeking, min_age, max_age, city,
                              interests, bio, photo_path, is_ai, persona_style)
        VALUES (:id, :name, :age, :gender, :seeking, :min_age, :max_age, :city,
                :interests, :bio, :photo_path, :is_ai, :persona_style)
        ON CONFLICT(id) DO UPDATE SET
            name = excluded.name, age = excluded.age, gender = excluded.gender,
            seeking = excluded.seeking, min_age = excluded.min_age, max_age = excluded.max_age,
            city = excluded.city, interests = excluded.interests, bio = excluded.bio,
            photo_path = excluded.photo_path, is_ai = excluded.is_ai,
            persona_style = excluded.persona_style
    )");
    query.bindValue(":id", profile.id);
    query.bindValue(":name", profile.name);
    query.bindValue(":age", profile.age);
    query.bindValue(":gender", int(profile.gender));
    query.bindValue(":seeking", int(profile.seeking));
    query.bindValue(":min_age", profile.minAge);
    query.bindValue(":max_age", profile.maxAge);
    query.bindValue(":city", profile.city);
    query.bindValue(":interests", interestsToJson(profile.interests));
    query.bindValue(":bio", profile.bio);
    query.bindValue(":photo_path", profile.photoPath);
    query.bindValue(":is_ai", profile.isAiPersona);
    query.bindValue(":persona_style", profile.personaStyle);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

std::optional<Profile> Database::profile(const QString &id) const
{
    const QList<Profile> found = queryProfiles("SELECT * FROM profiles WHERE id = :id", id);
    if (found.isEmpty())
        return std::nullopt;
    return found.first();
}

QList<Profile> Database::unseenProfiles(const QString &viewerId) const
{
    return queryProfiles(R"(
        SELECT * FROM profiles
        WHERE id != :id
          AND id NOT IN (SELECT to_id FROM swipes WHERE from_id = :id)
    )", viewerId);
}

bool Database::hasAiProfiles() const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    return query.exec("SELECT 1 FROM profiles WHERE is_ai = 1 LIMIT 1") && query.next();
}

QList<Profile> Database::queryProfiles(const QString &sql, const QString &bindId) const
{
    QList<Profile> profiles;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(sql);
    query.bindValue(":id", bindId);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return profiles;
    }
    while (query.next())
        profiles.append(profileFromQuery(query));
    return profiles;
}

// ---------- Hesaplar ----------

bool Database::createAccount(const QString &username, const QString &passwordHash, const Profile &profile)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction();

    if (!saveProfile(profile)) {
        db.rollback();
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO accounts (username, password_hash, profile_id) VALUES (:u, :h, :p)");
    query.bindValue(":u", username);
    query.bindValue(":h", passwordHash);
    query.bindValue(":p", profile.id);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        db.rollback();
        return false;
    }
    return db.commit();
}

std::optional<UserAccount> Database::account(const QString &username) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare("SELECT username, password_hash, profile_id FROM accounts WHERE username = :u");
    query.bindValue(":u", username);
    if (!query.exec() || !query.next())
        return std::nullopt;

    return UserAccount{query.value(0).toString(), query.value(1).toString(), query.value(2).toString()};
}

// ---------- Beğen / geç ve eşleşmeler ----------

bool Database::recordSwipe(const QString &fromId, const QString &toId, bool liked)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        INSERT INTO swipes (from_id, to_id, liked) VALUES (:from, :to, :liked)
        ON CONFLICT(from_id, to_id) DO UPDATE SET liked = excluded.liked
    )");
    query.bindValue(":from", fromId);
    query.bindValue(":to", toId);
    query.bindValue(":liked", liked);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::isMatch(const QString &userId, const QString &otherId) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT COUNT(*) FROM swipes
        WHERE liked = 1 AND ((from_id = :a AND to_id = :b) OR (from_id = :b AND to_id = :a))
    )");
    query.bindValue(":a", userId);
    query.bindValue(":b", otherId);
    return query.exec() && query.next() && query.value(0).toInt() == 2;
}

QList<Profile> Database::matches(const QString &userId) const
{
    return queryProfiles(R"(
        SELECT p.* FROM profiles p
        JOIN swipes mine   ON mine.to_id = p.id     AND mine.from_id = :id AND mine.liked = 1
        JOIN swipes theirs ON theirs.from_id = p.id AND theirs.to_id = :id AND theirs.liked = 1
        ORDER BY p.name
    )", userId);
}

bool Database::unmatch(const QString &userId, const QString &otherId)
{
    // Beğeni "geç" olarak işaretlenir: profil bir daha önerilmez, sohbet silinir
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction();

    if (!recordSwipe(userId, otherId, false)) {
        db.rollback();
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM messages WHERE user_id = :u AND other_id = :o");
    query.bindValue(":u", userId);
    query.bindValue(":o", otherId);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        db.rollback();
        return false;
    }
    return db.commit();
}

// ---------- Mesajlar ----------

bool Database::addMessage(const QString &userId, const QString &otherId, bool fromUser, const QString &text)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        INSERT INTO messages (user_id, other_id, from_user, text, sent_at)
        VALUES (:u, :o, :f, :t, :s)
    )");
    query.bindValue(":u", userId);
    query.bindValue(":o", otherId);
    query.bindValue(":f", fromUser);
    query.bindValue(":t", text);
    query.bindValue(":s", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QList<ChatMessage> Database::messages(const QString &userId, const QString &otherId) const
{
    QList<ChatMessage> result;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT from_user, text, sent_at FROM messages
        WHERE user_id = :u AND other_id = :o
        ORDER BY id
    )");
    query.bindValue(":u", userId);
    query.bindValue(":o", otherId);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return result;
    }
    while (query.next()) {
        ChatMessage message;
        message.fromUser = query.value(0).toBool();
        message.text = query.value(1).toString();
        message.sentAt = QDateTime::fromString(query.value(2).toString(), Qt::ISODate);
        result.append(message);
    }
    return result;
}
