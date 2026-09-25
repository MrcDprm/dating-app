#pragma once

#include "Profile.h"

#include <QList>

struct CompatibilityResult
{
    int score = 0;               // 0-100
    QStringList commonInterests;
    bool sameCity = false;
    int ageDifference = 0;
};

struct RankedProfile
{
    Profile profile;
    CompatibilityResult compatibility;
};

// İki kişi birbirinin cinsiyet ve yaş tercihine uyuyor mu (iki yönlü kontrol)
bool isMutualFit(const Profile &a, const Profile &b);

// İki profil arasındaki uyum puanı ve nedenleri
CompatibilityResult calculateCompatibility(const Profile &a, const Profile &b);

// Uygun adayları uyum puanına göre yüksekten düşüğe sıralar
QList<RankedProfile> rankCandidates(const Profile &me, const QList<Profile> &others);
