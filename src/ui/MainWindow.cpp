#include "MainWindow.h"

#include "DiscoverPage.h"
#include "data/Database.h"

#include <QLabel>
#include <QMessageBox>
#include <QTabWidget>

MainWindow::MainWindow(Database &db, const Profile &me, QWidget *parent)
    : QMainWindow(parent)
    , m_db(db)
    , m_me(me)
{
    setWindowTitle("Dating App - " + m_me.name);
    resize(1000, 720);

    m_discover = new DiscoverPage(m_db);
    m_discover->setCurrentUser(m_me);

    m_tabs = new QTabWidget;
    m_tabs->addTab(m_discover, "Keşfet");
    m_tabs->addTab(new QLabel("Eşleşmeler yakında"), "Eşleşmeler");
    m_tabs->addTab(new QLabel("Profil yakında"), "Profil");
    setCentralWidget(m_tabs);

    connect(m_discover, &DiscoverPage::matched, this, &MainWindow::showMatchDialog);
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
