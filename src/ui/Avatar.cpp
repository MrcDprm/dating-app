#include "Avatar.h"

#include <QColor>
#include <QFont>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>

QPixmap avatarPixmap(const Profile &profile, int size, qreal devicePixelRatio)
{
    QPixmap result(qRound(size * devicePixelRatio), qRound(size * devicePixelRatio));
    result.setDevicePixelRatio(devicePixelRatio);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath circle;
    circle.addEllipse(0, 0, size, size);
    painter.setClipPath(circle);

    const QPixmap photo(profile.photoPath);
    if (!profile.photoPath.isEmpty() && !photo.isNull()) {
        // Fotoğrafı kareyi dolduracak şekilde büyütüp ortadan kırp
        const QPixmap scaled = photo.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        painter.drawPixmap((size - scaled.width()) / 2, (size - scaled.height()) / 2, scaled);
        painter.end();
        return result;
    }

    // Aynı profil her açılışta aynı rengi alsın diye renk kimlikten hesaplanır
    int hue = 0;
    for (const QChar c : profile.id)
        hue = (hue * 31 + c.unicode()) % 360;
    painter.fillRect(0, 0, size, size, QColor::fromHsv(hue, 140, 210));

    QFont font = painter.font();
    font.setPixelSize(size * 2 / 5);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, QLocale(QLocale::Turkish).toUpper(profile.name.left(1)));
    painter.end();
    return result;
}
