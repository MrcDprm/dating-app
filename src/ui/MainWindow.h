#pragma once

#include "core/Profile.h"

#include <QMainWindow>

class Database;
class DiscoverPage;
class ProfileEditor;
class QTabWidget;

// Ana pencere: Keşfet, Eşleşmeler ve Profil sekmeleri
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Database &db, const Profile &me, const QString &dataDir, QWidget *parent = nullptr);

private:
    void showMatchDialog(const Profile &other);
    void onProfileSaved(const Profile &profile);

    Database &m_db;
    Profile m_me;
    QTabWidget *m_tabs = nullptr;
    DiscoverPage *m_discover = nullptr;
    ProfileEditor *m_profileEditor = nullptr;

};
