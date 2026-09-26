#pragma once

#include "core/Compatibility.h"

#include <QWidget>

class Database;
class QLabel;
class QPushButton;

// Keşfet sekmesi: uygun profilleri uyum puanına göre tek tek gösterir, beğen / geç
class DiscoverPage : public QWidget
{
    Q_OBJECT

public:
    explicit DiscoverPage(Database &db, QWidget *parent = nullptr);

    // Giriş yapan kullanıcıyı ayarlar ve aday listesini yeniden hesaplar
    void setCurrentUser(const Profile &me);

signals:
    void matched(const Profile &other);

private:
    void showCurrent();
    void swipe(bool liked);

    Database &m_db;
    Profile m_me;
    QList<RankedProfile> m_queue;

    QLabel *m_hint = nullptr;
    QWidget *m_card = nullptr;
    QLabel *m_avatar = nullptr;
    QLabel *m_title = nullptr;
    QLabel *m_score = nullptr;
    QLabel *m_details = nullptr;
    QLabel *m_bio = nullptr;
    QLabel *m_empty = nullptr;
};
