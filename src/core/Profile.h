#pragma once

#include <QString>
#include <QStringList>

// Uygulamanın kabul ettiği yaş sınırları
constexpr int kMinAge = 18;
constexpr int kMaxAge = 99;

enum class Gender { Man, Woman, Other };

// Kullanıcının görmek istediği profiller
enum class Seeking { Men, Women, Everyone };

struct Profile
{
    QString id;               // UUID: tahmin edilemeyen benzersiz kimlik
    QString name;
    int age = kMinAge;
    Gender gender = Gender::Other;
    Seeking seeking = Seeking::Everyone;
    int minAge = kMinAge;     // aradığı yaş aralığı
    int maxAge = kMaxAge;
    QString city;
    QStringList interests;
    QString bio;              // "Hakkımda" yazısı
    QString photoPath;        // boşsa varsayılan avatar gösterilir
    bool isAiPersona = false; // hazır profil mi (sohbete yapay zekâ cevap verir)
    QString personaStyle;     // yapay zekâ karakterinin konuşma tarzı
};

// Uygulamada seçilebilecek ilgi alanları (uyum puanı bunlar üzerinden hesaplanır)
inline const QStringList &interestCatalog()
{
    static const QStringList catalog = {
        "Müzik", "Sinema", "Kitap", "Seyahat", "Spor", "Doğa yürüyüşü",
        "Yemek", "Fotoğrafçılık", "Oyun", "Teknoloji", "Sanat", "Dans",
        "Hayvanlar", "Kahve", "Yoga", "Tiyatro", "Bilim", "Moda",
    };
    return catalog;
}
