#include "AboutDialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

const QString kRepoUrl = "https://github.com/MrcDprm/dating-app";

} // namespace

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Dating App Hakkında");
    setFixedWidth(440);

    auto *icon = new QLabel;
    icon->setPixmap(QIcon(":/icon.png").pixmap(72, 72));
    icon->setAlignment(Qt::AlignTop);

    auto *title = new QLabel("Dating App");
    title->setStyleSheet("font-size: 20px; font-weight: bold;");
    auto *version = new QLabel("Sürüm " + QApplication::applicationVersion());
    version->setStyleSheet("color: gray;");

    auto *description = new QLabel(
        "Uyum puanına göre eşleştirme yapan, eşleştiğin karakterlerle yapay zekâ destekli "
        "sohbet edebildiğin bir tanışma uygulaması.\n\n"
        "Hazır profillerin hepsi hayalidir ve sohbet cevapları yapay zekâ tarafından üretilir. "
        "Bu bir portfolyo projesidir; ağ üzerinden gerçek kullanıcılar yoktur.");
    description->setWordWrap(true);

    // Sabit içerik; kullanıcı verisi içermediği için bağlantı için zengin metin (HTML) güvenli
    auto *links = new QLabel(QString("<a href=\"%1\">GitHub'da kaynak kodu</a><br>"
                                     "MIT Lisansı · © 2026 Miraç Deprem<br>"
                                     "Qt 6 (LGPLv3), SQLite, libsodium").arg(kRepoUrl));
    links->setTextFormat(Qt::RichText);
    links->setOpenExternalLinks(true);

    auto *textColumn = new QVBoxLayout;
    textColumn->addWidget(title);
    textColumn->addWidget(version);
    textColumn->addSpacing(8);
    textColumn->addWidget(description);
    textColumn->addSpacing(8);
    textColumn->addWidget(links);

    auto *row = new QHBoxLayout;
    row->addWidget(icon);
    row->addSpacing(12);
    row->addLayout(textColumn, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    buttons->button(QDialogButtonBox::Close)->setText("Kapat");
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(row);
    layout->addWidget(buttons);
}
