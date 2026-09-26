#include "MainWindow.h"

#include "DiscoverPage.h"
#include "MatchesPage.h"
#include "ProfileEditor.h"
#include "SettingsDialog.h"
#include "ai/AiSettings.h"
#include "ai/ChatProvider.h"
#include "data/Database.h"
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>

MainWindow::MainWindow(Database &db, const Profile &me, const QString &dataDir, QWidget *parent)
    : QMainWindow(parent)
    , m_db(db)
    , m_me(me)
    , m_settingsFile(dataDir + "/settings.ini")
{
    setWindowTitle("Dating App - " + m_me.name);
    resize(1000, 720);

    m_provider = createChatProvider(AiSettings::load(m_settingsFile), this);


    m_discover = new DiscoverPage(m_db);
    m_discover->setCurrentUser(m_me);

    m_matches = new MatchesPage(m_db, *m_provider);
    m_matches->setCurrentUser(m_me);

    m_profileEditor = new ProfileEditor(m_db, dataDir + "/photos");
    m_profileEditor->setProfile(m_me);

    m_tabs = new QTabWidget;
    m_tabs->addTab(m_discover, "Keşfet");
    m_tabs->addTab(m_matches, "Eşleşmeler");
    m_tabs->addTab(m_profileEditor, "Profil");
    setCentralWidget(m_tabs);

        createMenus();

    connect(m_discover, &DiscoverPage::matched, this, &MainWindow::showMatchDialog);
    connect(m_profileEditor, &ProfileEditor::profileSaved, this, &MainWindow::onProfileSaved);

    // Yeni kullanıcı önce profilini tamamlasın
    if (m_me.interests.isEmpty())
        m_tabs->setCurrentWidget(m_profileEditor);
}

void MainWindow::onProfileSaved(const Profile &profile)
{
    m_me = profile;
    setWindowTitle("Dating App - " + m_me.name);
    m_discover->setCurrentUser(m_me); // yeni tercihlere göre adayları yeniden sırala
    m_matches->setCurrentUser(m_me);
}

void MainWindow::showMatchDialog(const Profile &other)
{
    QMessageBox box(this);
    box.setWindowTitle("Eşleşme!");
    box.setTextFormat(Qt::PlainText);
    box.setText(QString("%1 ile eşleştiniz! 🎉").arg(other.name));
    box.setIcon(QMessageBox::Information);
    QPushButton *chatButton = box.addButton("Sohbete başla", QMessageBox::AcceptRole);
    box.addButton("Keşfetmeye devam", QMessageBox::RejectRole);
    box.exec();

    m_matches->refresh();
    if (box.clickedButton() == chatButton) {
        m_tabs->setCurrentWidget(m_matches);
        m_matches->openChat(other.id);
    }
}


void MainWindow::createMenus()
{
    QMenu *accountMenu = menuBar()->addMenu("&Hesap");
    accountMenu->addAction("&Ayarlar…", QKeySequence("Ctrl+,"), this, &MainWindow::openSettings);
    accountMenu->addSeparator();
    accountMenu->addAction("Çıkış &yap", this, &MainWindow::logoutRequested);
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(AiSettings::load(m_settingsFile), this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (!dialog.settings().save(m_settingsFile)) {
        QMessageBox::warning(this, "Ayarlar", "Ayarlar kaydedilemedi.");
        return;
    }

    // Yeni sağlayıcıyı oluştur, sohbet ekranına ver, eskisini sil
    ChatProvider *oldProvider = m_provider;
    m_provider = createChatProvider(dialog.settings(), this);
    m_matches->setProvider(*m_provider);
    oldProvider->deleteLater();
}
