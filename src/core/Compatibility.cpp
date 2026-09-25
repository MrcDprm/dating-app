#include "Compatibility.h"

#include <algorithm>
#include <cstdlib>

namespace {

constexpr int kInterestWeight = 60;
constexpr int kAgeWeight = 25;
constexpr int kCityWeight = 15;
constexpr int kAgeGapLimit = 10; // 10 yıl ve üstü fark yaş puanını sıfırlar

bool wants(Seeking seeking, Gender gender)
{
    switch (seeking) {
    case Seeking::Men:
        return gender == Gender::Man;
    case Seeking::Women:
        return gender == Gender::Woman;
    case Seeking::Everyone:
        return true;
    }
    return false;
}

bool acceptsAgeOf(const Profile &viewer, const Profile &other)
{
    return other.age >= viewer.minAge && other.age <= viewer.maxAge;
}

} // namespace

bool isMutualFit(const Profile &a, const Profile &b)
{
    return wants(a.seeking, b.gender) && wants(b.seeking, a.gender)
        && acceptsAgeOf(a, b) && acceptsAgeOf(b, a);
}

CompatibilityResult calculateCompatibility(const Profile &a, const Profile &b)
{
    CompatibilityResult result;

    for (const QString &interest : a.interests) {
        if (b.interests.contains(interest) && !result.commonInterests.contains(interest))
            result.commonInterests.append(interest);
    }
    const qsizetype smaller = std::min(a.interests.size(), b.interests.size());
    const double interestRatio = smaller > 0 ? double(result.commonInterests.size()) / smaller : 0.0;

    result.ageDifference = std::abs(a.age - b.age);
    const double ageRatio = std::max(0.0, 1.0 - double(result.ageDifference) / kAgeGapLimit);

    result.sameCity = !a.city.isEmpty() && a.city.compare(b.city, Qt::CaseInsensitive) == 0;

    const double score = interestRatio * kInterestWeight + ageRatio * kAgeWeight
                       + (result.sameCity ? kCityWeight : 0);
    result.score = std::clamp(qRound(score), 0, 100);
    return result;
}

QList<RankedProfile> rankCandidates(const Profile &me, const QList<Profile> &others)
{
    QList<RankedProfile> ranked;
    for (const Profile &other : others) {
        if (other.id == me.id || !isMutualFit(me, other))
            continue;
        ranked.append({other, calculateCompatibility(me, other)});
    }

    std::stable_sort(ranked.begin(), ranked.end(), [](const RankedProfile &x, const RankedProfile &y) {
        return x.compatibility.score > y.compatibility.score;
    });
    return ranked;
}
