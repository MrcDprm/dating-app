#pragma once

#include "core/Profile.h"

#include <QList>
#include <QWidget>

class Database;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;

// Profil sekmesi: kullanıcının kendi profilini düzenlediği form
class ProfileEditor : public QWidget
{
    Q_OBJECT

public:
    ProfileEditor(Database &db, const QString &photosDir, QWidget *parent = nullptr);

    void setProfile(const Profile &profile);

signals:
    void profileSaved(const Profile &profile);

private:
    void choosePhoto();
    void removePhoto();
    void save();
    void updatePhotoPreview();

    Database &m_db;
    QString m_photosDir;
    Profile m_profile;
    QString m_savedPhotoPath; // veritabanındaki fotoğraf; kaldırılıp kaydedilince dosyası silinir

    QLabel *m_photoPreview = nullptr;
    QLineEdit *m_name = nullptr;
    QSpinBox *m_age = nullptr;
    QComboBox *m_gender = nullptr;
    QComboBox *m_seeking = nullptr;
    QSpinBox *m_minAge = nullptr;
    QSpinBox *m_maxAge = nullptr;
    QComboBox *m_city = nullptr;
    QList<QCheckBox *> m_interestBoxes;
    QPlainTextEdit *m_bio = nullptr;
    QLabel *m_message = nullptr;
};
