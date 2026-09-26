#pragma once

#include "data/Database.h"

#include <QObject>

// Yapay zekâ sağlayıcılarının ortak arayüzü (Ollama, Claude).
// Cevap beklenirken arayüz donmasın diye istek asenkron çalışır; sonuç sinyalle gelir.
class ChatProvider : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    ~ChatProvider() override = default;

    // requestId: cevabın hangi sohbete ait olduğunu ayırt etmek için (karşı profilin kimliği)
    virtual void requestReply(const QString &requestId, const QString &systemPrompt,
                              const QList<ChatMessage> &history) = 0;

signals:
    void replyReady(const QString &requestId, const QString &text);
    void failed(const QString &requestId, const QString &message);
};
