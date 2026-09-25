#include "LoginWindow.h"

#include "core/PasswordHasher.h"
#include "core/PasswordPolicy.h"
#include "data/Database.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>

#include <algorithm>

namespace {

constexpr int kMaxLoginAttempts = 5;
constexpr int kLockoutSeconds = 30;
constexpr int kMaxNameLength = 40;

QLabel *createErrorLabel()
{
    auto *label = new QLabel;
    label->setTextFormat(Qt::PlainText); // kullanıcı girdisi HTML olarak yorumlanmasın
    label->setWordWrap(true);
    label->setStyleSheet("color: #d93025;");
    label->hide();
    return label;
}

void showError(QLabel *label, const QString &message)
{
    label->setText(message);
    label->show();
}

} // namespace

LoginWindow::LoginWindow(Database &db, QWidget *parent)
    : QDialog(parent)
    , m_db(db)
{
    setWindowTitle("Dating App - Giriş");
    setMinimumWidth(380);

    auto *tabs = new QTabWidget;
    tabs->addTab(createLoginTab(), "Giriş yap");
    tabs->addTab(createRegisterTab(), "Kayıt ol");

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
}

QWidget *LoginWindow::createLoginTab()
{
    auto *page = new QWidget;
    m_loginUser = new QLineEdit;
    m_loginPassword = new QLineEdit;
    m_loginPassword->setEchoMode(QLineEdit::Password);
    m_loginButton = new QPushButton("Giriş yap");
    m_loginButton->setDefault(true);
    m_loginError = createErrorLabel();

    auto *form = new QFormLayout(page);
    form->addRow("Kullanıcı adı", m_loginUser);
    form->addRow("Şifre", m_loginPassword);
    form->addRow(m_loginError);
    form->addRow(m_loginButton);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::login);
    connect(m_loginPassword, &QLineEdit::returnPressed, this, &LoginWindow::login);
    return page;
}

QWidget *LoginWindow::createRegisterTab()
{
    auto *page = new QWidget;
    m_registerUser = new QLineEdit;
    m_registerUser->setPlaceholderText("harf, rakam, _ ve .");
    m_registerName = new QLineEdit;
    m_registerName->setMaxLength(kMaxNameLength);
    m_registerAge = new QSpinBox;
    m_registerAge->setRange(kMinAge, kMaxAge);
    m_registerAge->setValue(25);

    m_registerGender = new QComboBox;
    m_registerGender->addItem("Erkek", int(Gender::Man));
    m_registerGender->addItem("Kadın", int(Gender::Woman));
    m_registerGender->addItem("Diğer", int(Gender::Other));

    m_registerSeeking = new QComboBox;
    m_registerSeeking->addItem("Kadınlar", int(Seeking::Women));
    m_registerSeeking->addItem("Erkekler", int(Seeking::Men));
    m_registerSeeking->addItem("Herkes", int(Seeking::Everyone));

    m_registerPassword = new QLineEdit;
    m_registerPassword->setEchoMode(QLineEdit::Password);
    m_registerPassword->setPlaceholderText(QString("en az %1 karakter, büyük/küçük harf ve rakam").arg(kMinPasswordLength));
    m_registerPasswordAgain = new QLineEdit;
    m_registerPasswordAgain->setEchoMode(QLineEdit::Password);
    m_registerError = createErrorLabel();

    auto *button = new QPushButton("Hesap oluştur");

    auto *form = new QFormLayout(page);
    form->addRow("Kullanıcı adı", m_registerUser);
    form->addRow("Adın", m_registerName);
    form->addRow("Yaşın", m_registerAge);
    form->addRow("Cinsiyetin", m_registerGender);
    form->addRow("Kimi arıyorsun", m_registerSeeking);
    form->addRow("Şifre", m_registerPassword);
    form->addRow("Şifre (tekrar)", m_registerPasswordAgain);
    form->addRow(m_registerError);
    form->addRow(button);

    connect(button, &QPushButton::clicked, this, &LoginWindow::registerAccount);
    return page;
}

void LoginWindow::login()
{
    const QString username = m_loginUser->text().trimmed();
    const std::optional<UserAccount> account = m_db.account(username);

    if (account && PasswordHasher::verify(m_loginPassword->text(), account->passwordHash)) {
        m_profileId = account->profileId;
        accept();
        return;
    }

    // Hangi bilginin yanlış olduğu söylenmez: kullanıcı adlarının tahmin edilmesini zorlaştırır
    m_loginPassword->clear();
    if (++m_failedAttempts < kMaxLoginAttempts) {
        showError(m_loginError, "Kullanıcı adı veya şifre hatalı.");
        return;
    }

    showError(m_loginError, QString("Çok fazla hatalı deneme. %1 saniye bekle.").arg(kLockoutSeconds));
    m_loginButton->setEnabled(false);
    m_loginPassword->setEnabled(false);
    QTimer::singleShot(kLockoutSeconds * 1000, this, [this] {
        m_failedAttempts = 0;
        m_loginButton->setEnabled(true);
        m_loginPassword->setEnabled(true);
        m_loginError->hide();
    });
}

void LoginWindow::registerAccount()
{
    const QString username = m_registerUser->text().trimmed();
    const QString name = m_registerName->text().trimmed();
    const QString password = m_registerPassword->text();

    QStringList problems;
    static const QRegularExpression usernamePattern("^[A-Za-z0-9_.]{3,20}$");
    if (!usernamePattern.match(username).hasMatch())
        problems << "Kullanıcı adı 3-20 karakter olmalı; sadece harf, rakam, _ ve . içerebilir.";
    if (name.isEmpty())
        problems << "Adını gir.";
    problems << passwordProblems(password, username);
    if (password != m_registerPasswordAgain->text())
        problems << "Şifreler aynı değil.";

    if (!problems.isEmpty()) {
        showError(m_registerError, problems.join('\n'));
        return;
    }
    if (m_db.account(username)) {
        showError(m_registerError, "Bu kullanıcı adı alınmış.");
        return;
    }

    const std::optional<QString> hash = PasswordHasher::hash(password);
    if (!hash) {
        showError(m_registerError, "Şifre işlenemedi, tekrar dene.");
        return;
    }

    Profile profile;
    profile.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    profile.name = name;
    profile.age = m_registerAge->value();
    profile.gender = static_cast<Gender>(m_registerGender->currentData().toInt());
    profile.seeking = static_cast<Seeking>(m_registerSeeking->currentData().toInt());
    profile.minAge = std::max(kMinAge, profile.age - 7);
    profile.maxAge = std::min(kMaxAge, profile.age + 7);

    if (!m_db.createAccount(username, *hash, profile)) {
        showError(m_registerError, "Hesap oluşturulamadı. Lütfen tekrar dene.");
        return;
    }
    m_profileId = profile.id;
    accept();
}
