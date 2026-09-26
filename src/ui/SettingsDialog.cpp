#include "SettingsDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(const AiSettings &settings, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle("Ayarlar");
    setMinimumWidth(460);

    m_ollama = new QRadioButton("Ollama (bilgisayarında, ücretsiz)");
    m_claude = new QRadioButton("Claude API (kendi anahtarınla, ücretli, daha iyi Türkçe)");
    m_ollama->setChecked(settings.provider == AiSettings::Provider::Ollama);
    m_claude->setChecked(settings.provider == AiSettings::Provider::Claude);

    m_ollamaModel = new QLineEdit(settings.ollamaModel);
    m_claudeModel = new QLineEdit(settings.claudeModel);
    m_claudeKey = new QLineEdit;
    m_claudeKey->setEchoMode(QLineEdit::Password);
    m_claudeKey->setPlaceholderText(settings.claudeApiKey.isEmpty() ? "sk-ant-..." : "Kayıtlı (değiştirmek için yeni anahtar yaz)");
    auto *removeKeyButton = new QPushButton("Kayıtlı anahtarı sil");
    removeKeyButton->setEnabled(!settings.claudeApiKey.isEmpty());

    auto *note = new QLabel("Anahtar bu bilgisayarda Windows'un şifreleme sistemiyle (DPAPI) saklanır ve "
                            "sadece api.anthropic.com adresine gönderilir.");
    note->setWordWrap(true);
    note->setStyleSheet("color: gray; font-size: 11px;");

    auto *form = new QFormLayout;
    form->addRow("Ollama modeli", m_ollamaModel);
    form->addRow("Claude modeli", m_claudeModel);
    form->addRow("Claude API anahtarı", m_claudeKey);
    form->addRow("", removeKeyButton);

    auto *group = new QGroupBox("Sohbetlerde kullanılacak yapay zekâ");
    auto *groupLayout = new QVBoxLayout(group);
    groupLayout->addWidget(m_ollama);
    groupLayout->addWidget(m_claude);
    groupLayout->addLayout(form);
    groupLayout->addWidget(note);

    m_error = new QLabel;
    m_error->setTextFormat(Qt::PlainText);
    m_error->setStyleSheet("color: #d93025;");
    m_error->hide();

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText("Kaydet");
    buttons->button(QDialogButtonBox::Cancel)->setText("İptal");

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(group);
    layout->addWidget(m_error);
    layout->addWidget(buttons);

    connect(m_ollama, &QRadioButton::toggled, this, &SettingsDialog::updateEnabledFields);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    connect(removeKeyButton, &QPushButton::clicked, this, [this, removeKeyButton] {
        m_removeKey = true;
        m_claudeKey->clear();
        m_claudeKey->setPlaceholderText("sk-ant-...");
        removeKeyButton->setEnabled(false);
    });
    updateEnabledFields();
}

void SettingsDialog::updateEnabledFields()
{
    const bool useClaude = m_claude->isChecked();
    m_ollamaModel->setEnabled(!useClaude);
    m_claudeModel->setEnabled(useClaude);
    m_claudeKey->setEnabled(useClaude);
}

void SettingsDialog::accept()
{
    AiSettings updated = m_settings;
    updated.provider = m_claude->isChecked() ? AiSettings::Provider::Claude : AiSettings::Provider::Ollama;
    updated.ollamaModel = m_ollamaModel->text().trimmed();
    updated.claudeModel = m_claudeModel->text().trimmed();

    const QString newKey = m_claudeKey->text().trimmed();
    if (!newKey.isEmpty())
        updated.claudeApiKey = newKey;
    else if (m_removeKey)
        updated.claudeApiKey.clear();

    if (updated.ollamaModel.isEmpty() || updated.claudeModel.isEmpty()) {
        m_error->setText("Model adı boş olamaz.");
        m_error->show();
        return;
    }
    if (!newKey.isEmpty() && !newKey.startsWith("sk-ant-")) {
        m_error->setText("Bu bir Claude API anahtarına benzemiyor (\"sk-ant-\" ile başlamalı).");
        m_error->show();
        return;
    }
    if (updated.provider == AiSettings::Provider::Claude && updated.claudeApiKey.isEmpty()) {
        m_error->setText("Claude'u kullanmak için bir API anahtarı gir.");
        m_error->show();
        return;
    }

    m_settings = updated;
    QDialog::accept();
}
