#include "SeedProfiles.h"

#include "Database.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

#include <algorithm>
#include <optional>

namespace {

std::optional<Gender> parseGender(const QString &value)
{
    if (value == "man")
        return Gender::Man;
    if (value == "woman")
        return Gender::Woman;
    if (value == "other")
        return Gender::Other;
    return std::nullopt;
}

std::optional<Seeking> parseSeeking(const QString &value)
{
    if (value == "men")
        return Seeking::Men;
    if (value == "women")
        return Seeking::Women;
    if (value == "everyone")
        return Seeking::Everyone;
    return std::nullopt;
}

// Geçersiz bir kayıt (eksik alan, 18 yaş altı, bozuk kimlik) std::nullopt döner ve atlanır
std::optional<Profile> profileFromJson(const QJsonObject &object)
{
    Profile profile;
    profile.id = object.value("id").toString();
    profile.name = object.value("name").toString().trimmed();
    profile.age = object.value("age").toInt();

    const std::optional<Gender> gender = parseGender(object.value("gender").toString());
    const std::optional<Seeking> seeking = parseSeeking(object.value("seeking").toString());

    if (QUuid::fromString(profile.id).isNull() || profile.name.isEmpty()
        || profile.age < kMinAge || profile.age > kMaxAge || !gender || !seeking) {
        return std::nullopt;
    }

    profile.gender = *gender;
    profile.seeking = *seeking;
    profile.minAge = std::clamp(object.value("minAge").toInt(kMinAge), kMinAge, kMaxAge);
    profile.maxAge = std::clamp(object.value("maxAge").toInt(kMaxAge), profile.minAge, kMaxAge);
    profile.city = object.value("city").toString();
    profile.bio = object.value("bio").toString();
    profile.personaStyle = object.value("personaStyle").toString();
    profile.isAiPersona = true;

    for (const QJsonValue &interest : object.value("interests").toArray()) {
        if (interestCatalog().contains(interest.toString()))
            profile.interests.append(interest.toString());
    }
    return profile;
}

} // namespace

int importSeedProfiles(Database &db, const QByteArray &json)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(json, &error);
    if (error.error != QJsonParseError::NoError || !document.isArray())
        return -1;

    int imported = 0;
    for (const QJsonValue &value : document.array()) {
        const std::optional<Profile> profile = profileFromJson(value.toObject());
        if (profile && db.saveProfile(*profile))
            ++imported;
    }
    return imported;
}
