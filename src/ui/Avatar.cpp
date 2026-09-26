#include "Avatar.h"

#include <QColor>
#include <QFont>
#include <QImageReader>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>
#include <QTextBoundaryFinder>

namespace {

// Ekranda tek karakter görünen ilk parça (emoji ya da birleşik harf birden fazla QChar olabilir)
QString firstCharacter(const QString &text)
{
    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);
    const qsizetype end = finder.toNextBoundary();
    return end > 0 ? text.left(end) : QString("?");
}

} // namespace

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

    // QImageReader, QPixmap'in dosya önbelleğini kullanmaz; aynı adla değiştirilen fotoğraf hemen görünür
    const QPixmap photo = profile.photoPath.isEmpty() ? QPixmap() : QPixmap::fromImage(QImageReader(profile.photoPath).read());
    if (!photo.isNull()) {
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
    painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, QLocale(QLocale::Turkish).toUpper(firstCharacter(profile.name)));
    painter.end();
    return result;
}
