#pragma once

#include <QStringList>

constexpr int kMinPasswordLength = 8;
constexpr int kMaxPasswordLength = 128;

// Şifredeki eksikleri kullanıcıya gösterilecek mesajlar olarak döndürür.
// Boş liste, şifrenin kurallara uyduğu anlamına gelir.
QStringList passwordProblems(const QString &password, const QString &username);
