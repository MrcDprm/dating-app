#pragma once

#include <QDialog>

class Database;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;

// Giriş ve kayıt penceresi. Başarılı olursa accept() ile kapanır ve profileId() dolu olur.
class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(Database &db, QWidget *parent = nullptr);

    QString profileId() const { return m_profileId; }

private:
    QWidget *createLoginTab();
    QWidget *createRegisterTab();
    void login();
    void registerAccount();

    Database &m_db;
    QString m_profileId;
    int m_failedAttempts = 0;

    QLineEdit *m_loginUser = nullptr;
    QLineEdit *m_loginPassword = nullptr;
    QPushButton *m_loginButton = nullptr;
    QLabel *m_loginError = nullptr;

    QLineEdit *m_registerUser = nullptr;
    QLineEdit *m_registerName = nullptr;
    QSpinBox *m_registerAge = nullptr;
    QComboBox *m_registerGender = nullptr;
    QComboBox *m_registerSeeking = nullptr;
    QLineEdit *m_registerPassword = nullptr;
    QLineEdit *m_registerPasswordAgain = nullptr;
    QLabel *m_registerError = nullptr;
};
