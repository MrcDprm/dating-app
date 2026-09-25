#include "core/PasswordHasher.h"
#include "data/Database.h"
#include "data/SeedProfiles.h"
#include "ui/LoginWindow.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QStandardPaths>

namespace {

int fail(const QString &message)
{
    QMessageBox::critical(nullptr, "Dating App", message);
    return 1;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("DatingApp");
    QApplication::setApplicationVersion(APP_VERSION);

    if (!PasswordHasher::initialize())
        return fail("Güvenlik kütüphanesi başlatılamadı.");

    // Veriler program klasörüne değil kullanıcı klasörüne yazılır: %APPDATA%\DatingApp
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir().mkpath(dataDir))
        return fail("Veri klasörü oluşturulamadı:\n" + QDir::toNativeSeparators(dataDir));

    Database db;
    if (!db.open(dataDir + "/dating.db")) {
        qWarning() << "Veritabanı hatası:" << db.lastError(); // ayrıntı sadece geliştirici günlüğüne
        return fail("Veritabanı açılamadı.");
    }

    if (!db.hasAiProfiles()) {
        QFile seed(":/seed_profiles.json");
        if (!seed.open(QIODevice::ReadOnly) || importSeedProfiles(db, seed.readAll()) <= 0)
            qWarning() << "Hazır profiller yüklenemedi";
    }

    LoginWindow login(db);
    if (login.exec() != QDialog::Accepted)
        return 0;

    const std::optional<Profile> me = db.profile(login.profileId());
    if (!me)
        return fail("Profil bulunamadı.");

    QMainWindow window;
    window.setWindowTitle("Dating App");
    window.resize(1000, 700);
    auto *welcome = new QLabel("Hoş geldin, " + me->name + "!");
    welcome->setTextFormat(Qt::PlainText);
    window.setCentralWidget(welcome);
    window.show();

    return app.exec();
}
