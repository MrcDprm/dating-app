#pragma once

#include "core/Profile.h"

#include <QMainWindow>

class Database;
class DiscoverPage;
class QTabWidget;

// Ana pencere: Keşfet, Eşleşmeler ve Profil sekmeleri
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Database &db, const Profile &me, QWidget *parent = nullptr);

private:
    void showMatchDialog(const Profile &other);

    Database &m_db;
    Profile m_me;
    QTabWidget *m_tabs = nullptr;
    DiscoverPage *m_discover = nullptr;
};
