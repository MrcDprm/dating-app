#pragma once

#include <QString>

class ChatProvider;
class QObject;

// Yapay zekâ ayarları: %APPDATA%\DatingApp\settings.ini dosyasında saklanır.
// API anahtarı dosyaya düz metin değil, DPAPI ile şifrelenmiş olarak yazılır.
struct AiSettings
{
    enum class Provider { Ollama, Claude };

    Provider provider = Provider::Ollama;
    QString ollamaModel = "qwen2.5:7b-instruct";
    QString claudeModel = "claude-opus-5";
    QString claudeApiKey; // bellekte çözülmüş hâli; diske şifreli yazılır

    static AiSettings load(const QString &settingsFile);
    bool save(const QString &settingsFile) const;
};

// Ayarlara göre uygun sağlayıcıyı oluşturur; Claude seçili ama anahtar yoksa Ollama'ya düşer
ChatProvider *createChatProvider(const AiSettings &settings, QObject *parent);
