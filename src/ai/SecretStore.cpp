#include "SecretStore.h"

#include <windows.h>
#include <dpapi.h>

namespace SecretStore {

std::optional<QByteArray> encrypt(const QString &secret)
{
    QByteArray plain = secret.toUtf8();
    DATA_BLOB input{static_cast<DWORD>(plain.size()), reinterpret_cast<BYTE *>(plain.data())};
    DATA_BLOB output{};

    const BOOL ok = CryptProtectData(&input, L"DatingApp", nullptr, nullptr, nullptr,
                                     CRYPTPROTECT_UI_FORBIDDEN, &output);
    SecureZeroMemory(plain.data(), plain.size()); // düz metni bellekte bırakma
    if (!ok)
        return std::nullopt;

    QByteArray result(reinterpret_cast<const char *>(output.pbData), static_cast<qsizetype>(output.cbData));
    LocalFree(output.pbData);
    return result;
}

std::optional<QString> decrypt(const QByteArray &encrypted)
{
    QByteArray copy = encrypted;
    DATA_BLOB input{static_cast<DWORD>(copy.size()), reinterpret_cast<BYTE *>(copy.data())};
    DATA_BLOB output{};

    if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output))
        return std::nullopt;

    const QString result = QString::fromUtf8(reinterpret_cast<const char *>(output.pbData),
                                             static_cast<qsizetype>(output.cbData));
    SecureZeroMemory(output.pbData, output.cbData);
    LocalFree(output.pbData);
    return result;
}

} // namespace SecretStore
