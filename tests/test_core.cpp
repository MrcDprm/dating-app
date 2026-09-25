#include "core/Compatibility.h"
#include "core/PasswordHasher.h"
#include "core/PasswordPolicy.h"

#include <QTest>

namespace {

Profile makeProfile(const QString &id, int age, Gender gender, Seeking seeking,
                    const QString &city, const QStringList &interests)
{
    Profile profile;
    profile.id = id;
    profile.age = age;
    profile.gender = gender;
    profile.seeking = seeking;
    profile.city = city;
    profile.interests = interests;
    return profile;
}

} // namespace

class TestCore : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void identicalProfilesScoreFull();
    void nothingInCommonScoresZero();
    void duplicateInterestsCountOnce();
    void mutualFitChecksBothSides();
    void mutualFitChecksAgeRange();
    void rankingSortsAndFilters();
    void weakPasswordsAreRejected_data();
    void weakPasswordsAreRejected();
    void strongPasswordIsAccepted();
    void hashVerifiesOnlyCorrectPassword();

};

void TestCore::identicalProfilesScoreFull()
{
    const Profile a = makeProfile("a", 25, Gender::Man, Seeking::Women, "İzmir", {"Müzik", "Kahve"});
    const Profile b = makeProfile("b", 25, Gender::Woman, Seeking::Men, "İzmir", {"Müzik", "Kahve"});

    const CompatibilityResult result = calculateCompatibility(a, b);
    QCOMPARE(result.score, 100);
    QCOMPARE(result.commonInterests, QStringList({"Müzik", "Kahve"}));
    QVERIFY(result.sameCity);
}

void TestCore::nothingInCommonScoresZero()
{
    const Profile a = makeProfile("a", 20, Gender::Man, Seeking::Women, "Ankara", {"Spor"});
    const Profile b = makeProfile("b", 40, Gender::Woman, Seeking::Men, "İzmir", {"Kitap"});

    const CompatibilityResult result = calculateCompatibility(a, b);
    QCOMPARE(result.score, 0);
    QVERIFY(result.commonInterests.isEmpty());
    QCOMPARE(result.ageDifference, 20);
}

void TestCore::duplicateInterestsCountOnce()
{
    const Profile a = makeProfile("a", 30, Gender::Man, Seeking::Women, "Ankara", {"Müzik", "Müzik"});
    const Profile b = makeProfile("b", 30, Gender::Woman, Seeking::Men, "İzmir", {"Müzik"});

    const CompatibilityResult result = calculateCompatibility(a, b);
    QCOMPARE(result.commonInterests.size(), 1);
    QCOMPARE(result.score, 85); // ilgi 60 + yaş 25, şehir farklı
}

void TestCore::mutualFitChecksBothSides()
{
    const Profile man = makeProfile("a", 25, Gender::Man, Seeking::Women, "", {});
    Profile woman = makeProfile("b", 25, Gender::Woman, Seeking::Women, "", {});
    QVERIFY(!isMutualFit(man, woman)); // o kadınları arıyor

    woman.seeking = Seeking::Everyone;
    QVERIFY(isMutualFit(man, woman));
}

void TestCore::mutualFitChecksAgeRange()
{
    Profile a = makeProfile("a", 28, Gender::Man, Seeking::Everyone, "", {});
    const Profile b = makeProfile("b", 22, Gender::Woman, Seeking::Everyone, "", {});
    a.minAge = 25;
    a.maxAge = 30;
    QVERIFY(!isMutualFit(a, b));

    a.minAge = 22;
    QVERIFY(isMutualFit(a, b));
}

void TestCore::rankingSortsAndFilters()
{
    const Profile me = makeProfile("me", 25, Gender::Man, Seeking::Everyone, "İzmir", {"Müzik", "Kahve"});
    const QList<Profile> others = {
        me,                                                                         // kendisi: listelenmez
        makeProfile("low", 34, Gender::Woman, Seeking::Everyone, "Ankara", {"Spor"}),
        makeProfile("high", 25, Gender::Woman, Seeking::Everyone, "İzmir", {"Müzik"}),
        makeProfile("no", 25, Gender::Man, Seeking::Women, "İzmir", {"Müzik"}),     // kadınları arıyor
    };

    const QList<RankedProfile> ranked = rankCandidates(me, others);
    QCOMPARE(ranked.size(), 2);
    QCOMPARE(ranked[0].profile.id, QString("high"));
    QCOMPARE(ranked[1].profile.id, QString("low"));
}

void TestCore::initTestCase()
{
    QVERIFY(PasswordHasher::initialize());
}

void TestCore::weakPasswordsAreRejected_data()
{
    QTest::addColumn<QString>("password");
    QTest::newRow("kısa") << "Ab1";
    QTest::newRow("büyük harf yok") << "abcdefg1";
    QTest::newRow("küçük harf yok") << "ABCDEFG1";
    QTest::newRow("rakam yok") << "Abcdefgh";
    QTest::newRow("kullanıcı adı içeriyor") << "Mirac2026x";
}

void TestCore::weakPasswordsAreRejected()
{
    QFETCH(QString, password);
    QVERIFY(!passwordProblems(password, "mirac").isEmpty());
}

void TestCore::strongPasswordIsAccepted()
{
    QVERIFY(passwordProblems("Güçlü2026Şifre", "mirac").isEmpty());
}

void TestCore::hashVerifiesOnlyCorrectPassword()
{
    const std::optional<QString> hash = PasswordHasher::hash("Güçlü2026Şifre");
    QVERIFY(hash.has_value());
    QVERIFY(hash->startsWith("$argon2id$"));
    QVERIFY(PasswordHasher::verify("Güçlü2026Şifre", *hash));
    QVERIFY(!PasswordHasher::verify("güçlü2026şifre", *hash));
    QVERIFY(!PasswordHasher::verify("Güçlü2026Şifre", "bozuk-veri"));
}

QTEST_GUILESS_MAIN(TestCore)
#include "test_core.moc"
