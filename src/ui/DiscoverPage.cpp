#include "DiscoverPage.h"

#include "Avatar.h"
#include "data/Database.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QShortcut>
#include <QVBoxLayout>

namespace {

constexpr int kAvatarSize = 180;

QLabel *createTextLabel(Qt::Alignment alignment = Qt::AlignCenter)
{
    auto *label = new QLabel;
    label->setTextFormat(Qt::PlainText); // profil metinleri HTML olarak yorumlanmasın
    label->setWordWrap(true);
    label->setAlignment(alignment);
    return label;
}

} // namespace

DiscoverPage::DiscoverPage(Database &db, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
{
    m_hint = createTextLabel();
    m_hint->setText("İpucu: Profil sekmesinden şehrini ve ilgi alanlarını eklersen uyum puanları daha anlamlı olur.");
    m_hint->setStyleSheet("color: gray;");

    m_avatar = new QLabel;
    m_avatar->setAlignment(Qt::AlignCenter);
    m_avatar->setFixedHeight(kAvatarSize + 16);
    m_title = createTextLabel();
    m_title->setStyleSheet("font-size: 22px; font-weight: bold;");
    m_score = createTextLabel();
    m_score->setStyleSheet("font-size: 16px; color: #e0457b; font-weight: bold;");
    m_details = createTextLabel();
    m_bio = createTextLabel();
    m_bio->setStyleSheet("font-style: italic;");

    auto *passButton = new QPushButton("✕  Geç");
    auto *likeButton = new QPushButton("♥  Beğen");
    passButton->setToolTip("Sol ok tuşu");
    likeButton->setToolTip("Sağ ok tuşu");
    likeButton->setStyleSheet("background-color: #e0457b; color: white; padding: 8px 20px;");
    passButton->setStyleSheet("padding: 8px 20px;");

    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(passButton);
    buttons->addWidget(likeButton);
    buttons->addStretch();

    m_card = new QWidget;
    m_card->setMaximumWidth(460);
    auto *cardLayout = new QVBoxLayout(m_card);
    cardLayout->addWidget(m_avatar);
    cardLayout->addWidget(m_title);
    cardLayout->addWidget(m_score);
    cardLayout->addWidget(m_details);
    cardLayout->addWidget(m_bio);
    cardLayout->addSpacing(12);
    cardLayout->addLayout(buttons);

    auto *emptyText = createTextLabel();
    emptyText->setText("Şimdilik gösterecek kimse kalmadı.\n"
                       "Profil sekmesinden yaş aralığını ya da kimi aradığını genişletebilir "
                       "veya geçtiğin profillere tekrar bakabilirsin.");
    auto *passedAgainButton = new QPushButton("Geçtiklerimi tekrar göster");
    m_emptyState = new QWidget;
    auto *emptyLayout = new QVBoxLayout(m_emptyState);
    emptyLayout->addWidget(emptyText);
    emptyLayout->addWidget(passedAgainButton, 0, Qt::AlignHCenter);


    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_hint);
    layout->addStretch();
    layout->addWidget(m_card, 0, Qt::AlignHCenter);
    layout->addWidget(m_emptyState);
    layout->addStretch();

    connect(passButton, &QPushButton::clicked, this, [this] { swipe(false); });
    connect(likeButton, &QPushButton::clicked, this, [this] { swipe(true); });
    connect(passedAgainButton, &QPushButton::clicked, this, &DiscoverPage::showPassedAgain);


    auto *passShortcut = new QShortcut(Qt::Key_Left, this);
    auto *likeShortcut = new QShortcut(Qt::Key_Right, this);
    passShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    likeShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(passShortcut, &QShortcut::activated, this, [this] { swipe(false); });
    connect(likeShortcut, &QShortcut::activated, this, [this] { swipe(true); });
}

void DiscoverPage::setCurrentUser(const Profile &me)
{
    m_me = me;
    m_hint->setVisible(me.interests.isEmpty() || me.city.isEmpty());
    // Ağ olmadığı için diğer yerel hesaplar gerçek bir karşı taraf değildir; sadece hazır profiller önerilir
    QList<Profile> candidates = m_db.unseenProfiles(m_me.id);
    candidates.removeIf([](const Profile &profile) { return !profile.isAiPersona; });
    m_queue = rankCandidates(m_me, candidates);

    showCurrent();
}

void DiscoverPage::showCurrent()
{
    const bool hasCandidate = !m_queue.isEmpty();
    m_card->setVisible(hasCandidate);
    m_emptyState->setVisible(!hasCandidate);
    if (!hasCandidate)
        return;

    const Profile &other = m_queue.first().profile;
    const CompatibilityResult &match = m_queue.first().compatibility;

    m_avatar->setPixmap(avatarPixmap(other, kAvatarSize, devicePixelRatioF()));
    m_title->setText(QString("%1, %2").arg(other.name).arg(other.age));
    m_score->setText(QString("%%1 uyum").arg(match.score));

    QStringList details;
    details << (other.city.isEmpty() ? "Şehir belirtilmemiş" : other.city + (match.sameCity ? " (aynı şehir)" : ""));
    details << (match.commonInterests.isEmpty() ? "Ortak ilgi alanı yok"
                                                : "Ortak: " + match.commonInterests.join(", "));
    details << "İlgi alanları: " + other.interests.join(", ");
    m_details->setText(details.join('\n'));
    m_bio->setText(other.bio.isEmpty() ? QString() : "“" + other.bio + "”");
}

void DiscoverPage::swipe(bool liked)
{
    if (m_queue.isEmpty())
        return;

    const RankedProfile current = m_queue.takeFirst();
    const Profile &other = current.profile;
    m_db.recordSwipe(m_me.id, other.id, liked);

    if (liked) {
        // Karşı taraf seni daha önce beğendiyse hemen eşleşme; yoksa hazır profil
        // uyum puanına bağlı bir olasılıkla geri beğenir
        const int roll = QRandomGenerator::global()->bounded(100);
        if (!m_db.isMatch(m_me.id, other.id) && other.isAiPersona
            && personaLikesBack(current.compatibility.score, roll)) {
            m_db.recordSwipe(other.id, m_me.id, true);
        }
        if (m_db.isMatch(m_me.id, other.id))
            emit matched(other);
    }
    showCurrent();
}

void DiscoverPage::showPassedAgain()
{
    m_db.resetPasses(m_me.id);
    setCurrentUser(m_me);
}