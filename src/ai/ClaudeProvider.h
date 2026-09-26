#pragma once

#include "ChatProvider.h"

class QNetworkAccessManager;

// Anthropic Claude API'sine (api.anthropic.com) bağlanan sağlayıcı.
// Anahtar kullanıcının kendisine aittir; uygulamaya gömülü değildir.
class ClaudeProvider : public ChatProvider
{
    Q_OBJECT

public:
    ClaudeProvider(const QString &apiKey, const QString &model, QObject *parent = nullptr);

    void requestReply(const QString &requestId, const QString &systemPrompt,
                      const QList<ChatMessage> &history) override;

private:
    QNetworkAccessManager *m_network = nullptr;
    QString m_apiKey;
    QString m_model;
};
