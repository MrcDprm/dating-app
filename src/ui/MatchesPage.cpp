#include "MatchesPage.h"

#include "Avatar.h"
#include "ChatPage.h"
#include "data/Database.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>

namespace {

constexpr int kListAvatarSize = 40;

} // namespace

MatchesPage::MatchesPage(Database &db, ChatProvider &provider, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
{
    m_list = new QListWidget;
    m_list->setFixedWidth(240);
    m_list->setIconSize(QSize(kListAvatarSize, kListAvatarSize));
    m_list->setSpacing(2);

    m_placeholder = new QLabel;
    m_placeholder->setAlignment(Qt::AlignCenter);
    m_placeholder->setStyleSheet("color: gray;");
    m_chat = new ChatPage(m_db, provider);

    m_stack = new QStackedWidget;
    m_stack->addWidget(m_placeholder);
    m_stack->addWidget(m_chat);

    auto *layout = new QHBoxLayout(this);
    layout->addWidget(m_list);
    layout->addWidget(m_stack, 1);

    connect(m_list, &QListWidget::currentRowChanged, this, &MatchesPage::onSelectionChanged);
    connect(m_chat, &ChatPage::unmatched, this, &MatchesPage::refresh);
}

void MatchesPage::setCurrentUser(const Profile &me)
{
    m_me = me;
    m_chat->setCurrentUser(me);
    refresh();
}

void MatchesPage::refresh()
{
    // Seçili kişi yenilemeden sonra da seçili kalsın
    const QString selectedId = m_list->currentItem() ? m_list->currentItem()->data(Qt::UserRole).toString() : QString();

    m_matches = m_db.matches(m_me.id);
    m_list->blockSignals(true);
    m_list->clear();
    for (const Profile &match : m_matches) {
        auto *item = new QListWidgetItem(QIcon(avatarPixmap(match, kListAvatarSize, devicePixelRatioF())),
                                         QString("%1, %2").arg(match.name).arg(match.age));
        item->setData(Qt::UserRole, match.id);
        m_list->addItem(item);
    }
    m_list->blockSignals(false);

    m_placeholder->setText(m_matches.isEmpty()
        ? "Henüz eşleşmen yok.\nKeşfet sekmesinden beğenmeye başla!"
        : "Sohbet etmek için soldan bir eşleşme seç.");
    openChat(selectedId);
}

void MatchesPage::openChat(const QString &otherId)
{
    for (int row = 0; row < m_list->count(); ++row) {
        if (m_list->item(row)->data(Qt::UserRole).toString() == otherId) {
            m_list->blockSignals(true); // sohbet iki kez yüklenmesin
            m_list->setCurrentRow(row);
            m_list->blockSignals(false);
            onSelectionChanged();
            return;
        }
    }
    m_list->setCurrentRow(-1);
    m_stack->setCurrentWidget(m_placeholder);
}

void MatchesPage::setProvider(ChatProvider &provider)
{
    m_chat->setProvider(provider);
}

void MatchesPage::onSelectionChanged()
{
    const int row = m_list->currentRow();
    if (row < 0 || row >= m_matches.size()) {
        m_stack->setCurrentWidget(m_placeholder);
        return;
    }
    m_chat->openChat(m_matches[row]);
    m_stack->setCurrentWidget(m_chat);
}
