#pragma once

#include "core/Profile.h"

#include <QMainWindow>

class Database;
class ChatProvider;
class DiscoverPage;
class MatchesPage;
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
    ChatProvider *m_provider = nullptr;
    DiscoverPage *m_discover = nullptr;
    MatchesPage *m_matches = nullptr;
    ProfileEditor *m_profileEditor = nullptr;

};
