#include "ClaudeProvider.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <algorithm>

namespace {

const QUrl kMessagesUrl("https://api.anthropic.com/v1/messages");
constexpr int kTimeoutMs = 60000;
constexpr int kMaxHistoryMessages = 20;
constexpr int kMaxReplyLength = 1000;

// API, rollerin user / assistant diye sırayla gelmesini bekler; arka arkaya aynı roldeki
// mesajlar (ör. cevap alınamayınca tekrar yazılan mesaj) tek mesajda birleştirilir
QJsonArray buildMessages(const QList<ChatMessage> &history)
{
    QJsonArray messages;
    QString lastRole;
    const qsizetype first = std::max<qsizetype>(0, history.size() - kMaxHistoryMessages);
    for (qsizetype i = first; i < history.size(); ++i) {
        const QString role = history[i].fromUser ? "user" : "assistant";
        if (messages.isEmpty() && role == "assistant")
            continue; // sohbet kullanıcı mesajıyla başlamalı
        if (role == lastRole) {
            QJsonObject previous = messages.last().toObject();
            previous["content"] = previous["content"].toString() + "\n" + history[i].text;
            messages[messages.size() - 1] = previous;
        } else {
            messages.append(QJsonObject{{"role", role}, {"content", history[i].text}});
            lastRole = role;
        }
    }
    return messages;
}

} // namespace

ClaudeProvider::ClaudeProvider(const QString &apiKey, const QString &model, QObject *parent)
    : ChatProvider(parent)
    , m_network(new QNetworkAccessManager(this))
    , m_apiKey(apiKey)
    , m_model(model)
{
}

void ClaudeProvider::requestReply(const QString &requestId, const QString &systemPrompt,
                                  const QList<ChatMessage> &history)
{
    const QJsonObject body{
        {"model", m_model},
        {"max_tokens", 2000},
        {"system", systemPrompt},
        {"messages", buildMessages(history)},
        {"output_config", QJsonObject{{"effort", "low"}}}, // kısa sohbet cevapları için yeterli ve hızlı
        {"fallbacks", "default"}, // güvenlik filtresi reddederse sunucu uygun başka modelle dener
    };

    QNetworkRequest request(kMessagesUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-api-key", m_apiKey.toUtf8());
    request.setRawHeader("anthropic-version", "2023-06-01");
    request.setRawHeader("anthropic-beta", "server-side-fallback-2026-07-01");
    request.setTransferTimeout(kTimeoutMs);

    QNetworkReply *reply = m_network->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, requestId] {
        reply->deleteLater();

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == 401 || status == 403) {
            emit failed(requestId, "Claude API anahtarı geçersiz. Ayarlar'dan kontrol et.");
            return;
        }
        if (status == 429) {
            emit failed(requestId, "Çok fazla istek gönderildi. Biraz bekleyip tekrar dene.");
            return;
        }
        if (status >= 500) {
            emit failed(requestId, "Claude şu an yoğun. Biraz sonra tekrar dene.");
            return;
        }
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            emit failed(requestId, "Cevap çok uzun sürdü. Tekrar dene.");
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            // Ayrıntılı hata sadece geliştirici günlüğüne; kullanıcıya genel mesaj
            qWarning() << "Claude API hatası:" << status << reply->errorString();
            emit failed(requestId, "Claude API'ye ulaşılamadı. İnternet bağlantını kontrol et.");
            return;
        }

        const QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
        if (json.value("stop_reason").toString() == "refusal") {
            emit failed(requestId, "Bu mesaja cevap verilemedi. Farklı bir şey yazmayı dene.");
            return;
        }

        QString text;
        for (const QJsonValue &block : json.value("content").toArray()) {
            if (block.toObject().value("type").toString() == "text")
                text += block.toObject().value("text").toString();
        }
        text = text.trimmed();
        if (text.isEmpty()) {
            emit failed(requestId, "Yapay zekâdan boş cevap geldi. Tekrar dene.");
            return;
        }
        emit replyReady(requestId, text.left(kMaxReplyLength));
    });
}
