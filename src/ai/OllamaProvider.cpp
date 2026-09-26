#include "OllamaProvider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <algorithm>

namespace {

const QUrl kChatUrl("http://localhost:11434/api/chat");
constexpr int kTimeoutMs = 120000;      // model ilk kez yüklenirken yavaş olabilir
constexpr int kMaxHistoryMessages = 20; // uzun sohbetlerde sadece son mesajlar gönderilir
constexpr int kMaxReplyLength = 1000;

QJsonObject message(const QString &role, const QString &content)
{
    return QJsonObject{{"role", role}, {"content", content}};
}

} // namespace

OllamaProvider::OllamaProvider(const QString &model, QObject *parent)
    : ChatProvider(parent)
    , m_network(new QNetworkAccessManager(this))
    , m_model(model)
{
}

void OllamaProvider::requestReply(const QString &requestId, const QString &systemPrompt,
                                  const QList<ChatMessage> &history)
{
    QJsonArray messages;
    messages.append(message("system", systemPrompt));
    const qsizetype first = std::max<qsizetype>(0, history.size() - kMaxHistoryMessages);
    for (qsizetype i = first; i < history.size(); ++i)
        messages.append(message(history[i].fromUser ? "user" : "assistant", history[i].text));

    const QJsonObject body{
        {"model", m_model},
        {"messages", messages},
        {"stream", false},
        {"options", QJsonObject{{"temperature", 0.7}, {"num_predict", 150}}},
    };

    QNetworkRequest request(kChatUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(kTimeoutMs);

    QNetworkReply *reply = m_network->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, requestId] {
        reply->deleteLater();

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() == QNetworkReply::ConnectionRefusedError) {
            emit failed(requestId, "Ollama çalışmıyor. Ollama'yı başlatıp tekrar dene.");
            return;
        }
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            emit failed(requestId, "Cevap çok uzun sürdü. Tekrar dene.");
            return;
        }
        if (status == 404) {
            emit failed(requestId, QString("\"%1\" modeli yüklü değil. Terminalde şunu çalıştır: ollama pull %1").arg(m_model));
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            emit failed(requestId, "Yapay zekâya ulaşılamadı. Tekrar dene.");
            return;
        }

        const QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
        const QString text = json.value("message").toObject().value("content").toString().trimmed();
        if (text.isEmpty()) {
            emit failed(requestId, "Yapay zekâdan boş cevap geldi. Tekrar dene.");
            return;
        }
        emit replyReady(requestId, text.left(kMaxReplyLength));
    });
}
