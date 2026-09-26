#include "MainWindow.h"

#include "DiscoverPage.h"
#include "ProfileEditor.h"
#include "data/Database.h"

#include <QLabel>
#include <QMessageBox>
#include <QTabWidget>

MainWindow::MainWindow(Database &db, const Profile &me, const QString &dataDir, QWidget *parent)
    : QMainWindow(parent)
    , m_db(db)
    , m_me(me)
{
    setWindowTitle("Dating App - " + m_me.name);
    resize(1000, 720);

    m_discover = new DiscoverPage(m_db);
    m_discover->setCurrentUser(m_me);    
    m_profileEditor = new ProfileEditor(m_db, dataDir + "/photos");
    m_profileEditor->setProfile(m_me);
    m_tabs = new QTabWidget;
    m_tabs->addTab(m_discover, "Keşfet");
    m_tabs->addTab(new QLabel("Eşleşmeler yakında"), "Eşleşmeler");
    m_tabs->addTab(m_profileEditor, "Profil");
    setCentralWidget(m_tabs);

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
}

void MainWindow::showMatchDialog(const Profile &other)
{
    QMessageBox box(this);
    box.setWindowTitle("Eşleşme!");
    box.setTextFormat(Qt::PlainText);
    box.setText(QString("%1 ile eşleştiniz! 🎉\nEşleşmeler sekmesinden sohbete başlayabilirsin.").arg(other.name));
    box.setIcon(QMessageBox::Information);
    box.exec();
}
