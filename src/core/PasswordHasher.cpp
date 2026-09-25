#include "PasswordHasher.h"

#include <sodium.h>

namespace PasswordHasher {

bool initialize()
{
    return sodium_init() >= 0;
}

std::optional<QString> hash(const QString &password)
{
    const QByteArray utf8 = password.toUtf8();
    char result[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(result, utf8.constData(), static_cast<unsigned long long>(utf8.size()),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        return std::nullopt; // bellek yetersiz
    }
    return QString::fromLatin1(result);
}

bool verify(const QString &password, const QString &storedHash)
{
    const QByteArray utf8 = password.toUtf8();
    const QByteArray stored = storedHash.toLatin1();

    return crypto_pwhash_str_verify(stored.constData(), utf8.constData(),
                                    static_cast<unsigned long long>(utf8.size())) == 0;
}

} // namespace PasswordHasher
