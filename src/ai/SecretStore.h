#pragma once

#include <QByteArray>
#include <QString>

#include <optional>

// API anahtarı gibi gizli bilgileri Windows DPAPI ile şifreler.
// Şifreli veri sadece aynı Windows kullanıcısı tarafından çözülebilir.
namespace SecretStore {

std::optional<QByteArray> encrypt(const QString &secret);
std::optional<QString> decrypt(const QByteArray &encrypted);

} // namespace SecretStore
