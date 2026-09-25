#pragma once

#include <QString>

#include <optional>

namespace PasswordHasher {

// Uygulama açılışında bir kez çağrılır; libsodium başlatılamazsa false döner
bool initialize();

// Şifrenin Argon2id özetini döndürür (tuz ve ayarlar özetin içindedir)
std::optional<QString> hash(const QString &password);

// Girilen şifre saklanan özetle eşleşiyor mu
bool verify(const QString &password, const QString &storedHash);

} // namespace PasswordHasher
