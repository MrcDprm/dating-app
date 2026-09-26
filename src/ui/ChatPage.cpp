#include "ChatPage.h"

#include "Avatar.h"
#include "ai/ChatProvider.h"
#include "ai/PersonaPrompt.h"
#include "data/Database.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace {

constexpr int kHeaderAvatarSize = 48;
constexpr int kMaxMessageLength = 500;
constexpr int kMaxBubbleWidth = 420;

} // namespace

ChatPage::ChatPage(Database &db, ChatProvider &provider, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
{
    // Üst bilgi: avatar, ad ve "eşleşmeyi kaldır"
    m_avatar = new QLabel;
    m_avatar->setFixedSize(kHeaderAvatarSize, kHeaderAvatarSize);
    m_title = new QLabel;
    m_title->setTextFormat(Qt::PlainText);
    m_title->setStyleSheet("font-size: 16px; font-weight: bold;");
    auto *aiBadge = new QLabel("Yapay zekâ karakteri");
    aiBadge->setStyleSheet("color: gray; font-size: 11px;");
    auto *titleColumn = new QVBoxLayout;
    titleColumn->addWidget(m_title);
    titleColumn->addWidget(aiBadge);
    auto *unmatchButton = new QPushButton("Eşleşmeyi kaldır");

    auto *header = new QHBoxLayout;
    header->addWidget(m_avatar);
    header->addLayout(titleColumn);
    header->addStretch();
    header->addWidget(unmatchButton);

    // Mesaj baloncukları kaydırılabilir bir alanda alt alta dizilir
    auto *bubbleContainer = new QWidget;
    m_bubbles = new QVBoxLayout(bubbleContainer);
    m_bubbles->addStretch();
    m_scroll = new QScrollArea;
    m_scroll->setWidgetResizable(true);
    m_scroll->setWidget(bubbleContainer);

    m_typing = new QLabel;
    m_typing->setTextFormat(Qt::PlainText);
    m_typing->setStyleSheet("color: gray; font-style: italic;");
    m_typing->hide();

    m_error = new QLabel;
    m_error->setTextFormat(Qt::PlainText);
    m_error->setWordWrap(true);
    m_error->setStyleSheet("color: #d93025;");
    m_retry = new QPushButton("Tekrar dene");
    auto *errorRow = new QHBoxLayout;
    errorRow->addWidget(m_error, 1);
    errorRow->addWidget(m_retry);
    m_error->hide();
    m_retry->hide();

    m_input = new QLineEdit;
    m_input->setMaxLength(kMaxMessageLength);
    m_input->setPlaceholderText("Bir mesaj yaz…");
    m_send = new QPushButton("Gönder");
    m_send->setStyleSheet("background-color: #e0457b; color: white; padding: 6px 18px;");
    auto *inputRow = new QHBoxLayout;
    inputRow->addWidget(m_input);
    inputRow->addWidget(m_send);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(header);
    layout->addWidget(m_scroll, 1);
    layout->addWidget(m_typing);
    layout->addLayout(errorRow);
    layout->addLayout(inputRow);

    connect(m_send, &QPushButton::clicked, this, &ChatPage::sendMessage);
    connect(m_input, &QLineEdit::returnPressed, this, &ChatPage::sendMessage);
    connect(m_retry, &QPushButton::clicked, this, &ChatPage::requestReply);
    connect(unmatchButton, &QPushButton::clicked, this, &ChatPage::confirmUnmatch);
    setProvider(provider);
}

void ChatPage::setProvider(ChatProvider &provider)
{
    // Eski sağlayıcıdan gelecek cevaplar artık dinlenmez; bekleyen istekler iptal sayılır
    if (m_provider)
        disconnect(m_provider, nullptr, this, nullptr);
    m_provider = &provider;
    m_waitingFor.clear();
    connect(m_provider, &ChatProvider::replyReady, this, &ChatPage::onReplyReady);
    connect(m_provider, &ChatProvider::failed, this, &ChatPage::onReplyFailed);
    updateInputState();
}


void ChatPage::setCurrentUser(const Profile &me)
{
    m_me = me;
}

void ChatPage::openChat(const Profile &other)
{
    m_other = other;
    m_avatar->setPixmap(avatarPixmap(other, kHeaderAvatarSize, devicePixelRatioF()));
    m_title->setText(QString("%1, %2").arg(other.name).arg(other.age));
    m_error->hide();
    m_retry->hide();
    reloadMessages();
    updateInputState();
    m_input->setFocus();
}

void ChatPage::sendMessage()
{
    const QString text = m_input->text().trimmed();
    if (text.isEmpty() || m_waitingFor.contains(m_other.id))
        return;

    if (!m_db.addMessage(m_me.id, m_other.id, true, text)) {
        m_error->setText("Mesaj kaydedilemedi.");
        m_error->show();
        return;
    }
    m_input->clear();
    addBubble(text, true);
    requestReply();
}

void ChatPage::requestReply()
{
    m_error->hide();
    m_retry->hide();
    m_waitingFor.insert(m_other.id);
    updateInputState();
    m_provider->requestReply(m_other.id, buildPersonaPrompt(m_other, m_me), m_db.messages(m_me.id, m_other.id));
}

void ChatPage::onReplyReady(const QString &otherId, const QString &text)
{
    m_waitingFor.remove(otherId);
    if (!m_db.isMatch(m_me.id, otherId))
        return; // beklerken eşleşme kaldırıldıysa cevap atılır

    // Kullanıcı başka sohbete geçmiş olsa bile cevap doğru sohbete kaydedilir
    m_db.addMessage(m_me.id, otherId, false, text);
    if (otherId == m_other.id) {
        addBubble(text, false);
        updateInputState();
    }
}

void ChatPage::onReplyFailed(const QString &otherId, const QString &message)
{
    m_waitingFor.remove(otherId);
    if (otherId != m_other.id)
        return;
    m_error->setText(message);
    m_error->show();
    m_retry->show();
    updateInputState();
}

void ChatPage::confirmUnmatch()
{
    QMessageBox box(this);
    box.setWindowTitle("Eşleşmeyi kaldır");
    box.setTextFormat(Qt::PlainText);
    box.setText(QString("%1 ile eşleşmeyi kaldırmak istiyor musun?\nSohbet geçmişi de silinecek.").arg(m_other.name));
    box.setIcon(QMessageBox::Warning);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    box.setDefaultButton(QMessageBox::Cancel);
    if (box.exec() != QMessageBox::Yes)
        return;

    if (m_db.unmatch(m_me.id, m_other.id))
        emit unmatched(m_other.id);
}

void ChatPage::reloadMessages()
{
    // Eski baloncukları sil (en alttaki esneme payı hariç)
    while (m_bubbles->count() > 1) {
        QLayoutItem *item = m_bubbles->takeAt(0);
        delete item->widget();
        delete item;
    }

    const QList<ChatMessage> messages = m_db.messages(m_me.id, m_other.id);
    if (messages.isEmpty()) {
        auto *note = new QLabel(QString("%1 ile eşleştiniz. İlk mesajı sen gönder!").arg(m_other.name));
        note->setTextFormat(Qt::PlainText);
        note->setAlignment(Qt::AlignCenter);
        note->setStyleSheet("color: gray;");
        m_bubbles->insertWidget(0, note);
    }
    for (const ChatMessage &message : messages)
        addBubble(message.text, message.fromUser);
}

void ChatPage::updateInputState()
{
    const bool waiting = m_waitingFor.contains(m_other.id);
    m_send->setEnabled(!waiting);
    m_typing->setText(QString("%1 yazıyor…").arg(m_other.name));
    m_typing->setVisible(waiting);
}

void ChatPage::addBubble(const QString &text, bool fromUser)
{
    auto *bubble = new QLabel(text);
    bubble->setTextFormat(Qt::PlainText); // mesajlar asla HTML olarak yorumlanmaz
    bubble->setWordWrap(true);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // Kelime kaydırmalı QLabel kendini gereğinden dar hesaplar; metin genişliğine göre en az genişlik ver
    bubble->setMinimumWidth(std::min(kMaxBubbleWidth, bubble->fontMetrics().horizontalAdvance(text) + 30));
    bubble->setMaximumWidth(kMaxBubbleWidth);
    bubble->setStyleSheet(fromUser
        ? "background-color: #e0457b; color: white; border-radius: 12px; padding: 8px 12px;"
        : "background-color: #3a3a3a; color: white; border-radius: 12px; padding: 8px 12px;");

    auto *row = new QHBoxLayout;
    if (fromUser)
        row->addStretch();
    row->addWidget(bubble);
    if (!fromUser)
        row->addStretch();

    auto *rowWidget = new QWidget;
    rowWidget->setLayout(row);
    m_bubbles->insertWidget(m_bubbles->count() - 1, rowWidget);

    // Yeni mesaj eklenince en alta kaydır (düzen güncellendikten sonra)
    QTimer::singleShot(0, this, [this] {
        m_scroll->verticalScrollBar()->setValue(m_scroll->verticalScrollBar()->maximum());
    });
}
