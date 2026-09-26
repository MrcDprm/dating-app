#pragma once

#include "ai/AiSettings.h"

#include <QDialog>

class QLabel;
class QLineEdit;
class QRadioButton;

// Ayarlar penceresi: yapay zekâ sağlayıcısı, model adları ve Claude API anahtarı
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(const AiSettings &settings, QWidget *parent = nullptr);

    AiSettings settings() const { return m_settings; }

private:
    void accept() override;
    void updateEnabledFields();

    AiSettings m_settings;
    QRadioButton *m_ollama = nullptr;
    QRadioButton *m_claude = nullptr;
    QLineEdit *m_ollamaModel = nullptr;
    QLineEdit *m_claudeModel = nullptr;
    QLineEdit *m_claudeKey = nullptr;
    QLabel *m_error = nullptr;
    bool m_removeKey = false;
};
