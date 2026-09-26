#pragma once

#include "ChatProvider.h"

class QNetworkAccessManager;

// Bilgisayarda çalışan Ollama sunucusuna (localhost:11434) bağlanan sağlayıcı
class OllamaProvider : public ChatProvider
{
    Q_OBJECT

public:
    explicit OllamaProvider(const QString &model, QObject *parent = nullptr);

    void requestReply(const QString &requestId, const QString &systemPrompt,
                      const QList<ChatMessage> &history) override;

private:
    QNetworkAccessManager *m_network = nullptr;
    QString m_model;
};
