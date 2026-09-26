#include "core/PasswordHasher.h"
#include "data/Database.h"
#include "data/SeedProfiles.h"
#include "ui/LoginWindow.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
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

       // Hazır profiller her açılışta güncellenir; yeni sürümde eklenen profiller de gelir
    QFile seed(":/seed_profiles.json");
    if (!seed.open(QIODevice::ReadOnly) || importSeedProfiles(db, seed.readAll()) <= 0)
        qWarning() << "Hazır profiller yüklenemedi";


    // "Çıkış yap" seçilirse ana pencere kapanır ve giriş ekranı tekrar açılır
    while (true) {
        LoginWindow login(db);
        if (login.exec() != QDialog::Accepted)
            return 0;

        const std::optional<Profile> me = db.profile(login.profileId());
        if (!me)
            return fail("Profil bulunamadı.");

        MainWindow window(db, *me, dataDir);
        bool loggedOut = false;
        QObject::connect(&window, &MainWindow::logoutRequested, &window, [&] {
            loggedOut = true;
            window.close();
        });
        window.show();

        const int exitCode = app.exec();
        if (!loggedOut)
            return exitCode;
    }
}

