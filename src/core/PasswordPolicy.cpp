#include "PasswordPolicy.h"

#include <algorithm>

QStringList passwordProblems(const QString &password, const QString &username)
{
    QStringList problems;

    if (password.size() < kMinPasswordLength)
        problems << QString("En az %1 karakter olmalı.").arg(kMinPasswordLength);
    if (password.size() > kMaxPasswordLength)
        problems << QString("En fazla %1 karakter olabilir.").arg(kMaxPasswordLength);

    const auto hasAny = [&password](auto predicate) {
        return std::any_of(password.begin(), password.end(), predicate);
    };
    if (!hasAny([](QChar c) { return c.isUpper(); }))
        problems << "En az bir büyük harf içermeli.";
    if (!hasAny([](QChar c) { return c.isLower(); }))
        problems << "En az bir küçük harf içermeli.";
    if (!hasAny([](QChar c) { return c.isDigit(); }))
        problems << "En az bir rakam içermeli.";

    if (!username.isEmpty() && password.contains(username, Qt::CaseInsensitive))
        problems << "Kullanıcı adını içermemeli.";

    return problems;
}
