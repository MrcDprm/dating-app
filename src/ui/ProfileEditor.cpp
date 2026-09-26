#include "ProfileEditor.h"

#include "Avatar.h"
#include "data/Database.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {

constexpr int kPreviewSize = 120;
constexpr int kMaxInterests = 8;
constexpr int kMaxBioLength = 300;
constexpr qint64 kMaxPhotoBytes = 5 * 1024 * 1024; // 5 MB
constexpr int kMaxPhotoDimension = 8000;            // aşırı büyük resimler belleği doldurmasın
constexpr int kStoredPhotoSize = 512;

const QStringList &cityCatalog()
{
    static const QStringList cities = {
        "Adana", "Adıyaman", "Afyonkarahisar", "Ağrı", "Aksaray", "Amasya", "Ankara", "Antalya",
        "Ardahan", "Artvin", "Aydın", "Balıkesir", "Bartın", "Batman", "Bayburt", "Bilecik",
        "Bingöl", "Bitlis", "Bolu", "Burdur", "Bursa", "Çanakkale", "Çankırı", "Çorum",
        "Denizli", "Diyarbakır", "Düzce", "Edirne", "Elazığ", "Erzincan", "Erzurum", "Eskişehir",
        "Gaziantep", "Giresun", "Gümüşhane", "Hakkari", "Hatay", "Iğdır", "Isparta", "İstanbul",
        "İzmir", "Kahramanmaraş", "Karabük", "Karaman", "Kars", "Kastamonu", "Kayseri", "Kilis",
        "Kırıkkale", "Kırklareli", "Kırşehir", "Kocaeli", "Konya", "Kütahya", "Malatya", "Manisa",
        "Mardin", "Mersin", "Muğla", "Muş", "Nevşehir", "Niğde", "Ordu", "Osmaniye",
        "Rize", "Sakarya", "Samsun", "Şanlıurfa", "Siirt", "Sinop", "Şırnak", "Sivas",
        "Tekirdağ", "Tokat", "Trabzon", "Tunceli", "Uşak", "Van", "Yalova", "Yozgat", "Zonguldak",
    };
    return cities;
}

QLabel *createMessageLabel()
{
    auto *label = new QLabel;
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->hide();
    return label;
}

void showMessage(QLabel *label, const QString &text, bool isError)
{
    label->setStyleSheet(isError ? "color: #d93025;" : "color: #188038;");
    label->setText(text);
    label->show();
}

} // namespace

ProfileEditor::ProfileEditor(Database &db, const QString &photosDir, QWidget *parent)
    : QWidget(parent)
    , m_db(db)
    , m_photosDir(photosDir)
{
    // Fotoğraf
    m_photoPreview = new QLabel;
    m_photoPreview->setFixedSize(kPreviewSize, kPreviewSize);
    auto *choosePhotoButton = new QPushButton("Fotoğraf seç…");
    auto *removePhotoButton = new QPushButton("Fotoğrafı kaldır");
    auto *photoButtons = new QVBoxLayout;
    photoButtons->addWidget(choosePhotoButton);
    photoButtons->addWidget(removePhotoButton);
    photoButtons->addStretch();
    auto *photoRow = new QHBoxLayout;
    photoRow->addWidget(m_photoPreview);
    photoRow->addLayout(photoButtons);
    photoRow->addStretch();

    // Temel bilgiler
    m_name = new QLineEdit;
    m_name->setMaxLength(40);
    m_age = new QSpinBox;
    m_age->setRange(kMinAge, kMaxAge);
    m_gender = new QComboBox;
    m_gender->addItem("Erkek", int(Gender::Man));
    m_gender->addItem("Kadın", int(Gender::Woman));
    m_gender->addItem("Diğer", int(Gender::Other));
    m_seeking = new QComboBox;
    m_seeking->addItem("Kadınlar", int(Seeking::Women));
    m_seeking->addItem("Erkekler", int(Seeking::Men));
    m_seeking->addItem("Herkes", int(Seeking::Everyone));
    m_minAge = new QSpinBox;
    m_minAge->setRange(kMinAge, kMaxAge);
    m_maxAge = new QSpinBox;
    m_maxAge->setRange(kMinAge, kMaxAge);
    auto *ageRange = new QHBoxLayout;
    ageRange->addWidget(m_minAge);
    ageRange->addWidget(new QLabel("-"));
    ageRange->addWidget(m_maxAge);
    ageRange->addStretch();
    m_city = new QComboBox;
    m_city->addItem("Seçilmedi", QString());
    for (const QString &city : cityCatalog())
        m_city->addItem(city, city);

    auto *form = new QFormLayout;
    form->addRow("Adın", m_name);
    form->addRow("Yaşın", m_age);
    form->addRow("Cinsiyetin", m_gender);
    form->addRow("Kimi arıyorsun", m_seeking);
    form->addRow("Aradığın yaş aralığı", ageRange);
    form->addRow("Şehir", m_city);

    // İlgi alanları
    auto *interestsBox = new QGroupBox(QString("İlgi alanları (en fazla %1)").arg(kMaxInterests));
    auto *interestsGrid = new QGridLayout(interestsBox);
    for (int i = 0; i < interestCatalog().size(); ++i) {
        auto *box = new QCheckBox(interestCatalog()[i]);
        interestsGrid->addWidget(box, i / 3, i % 3);
        m_interestBoxes.append(box);
    }

    // Hakkında
    m_bio = new QPlainTextEdit;
    m_bio->setPlaceholderText(QString("Kendini birkaç cümleyle anlat (en fazla %1 karakter)").arg(kMaxBioLength));
    m_bio->setMaximumHeight(100);

    m_message = createMessageLabel();
    auto *saveButton = new QPushButton("Kaydet");
    saveButton->setStyleSheet("background-color: #e0457b; color: white; padding: 6px 24px;");

    auto *content = new QWidget;
    content->setMaximumWidth(640);
    auto *layout = new QVBoxLayout(content);
    layout->addLayout(photoRow);
    layout->addLayout(form);
    layout->addWidget(interestsBox);
    layout->addWidget(new QLabel("Hakkında"));
    layout->addWidget(m_bio);
    layout->addWidget(m_message);
    layout->addWidget(saveButton, 0, Qt::AlignLeft);
    layout->addStretch();

    // Küçük pencerelerde form kaydırılabilsin
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(content);
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);

    connect(choosePhotoButton, &QPushButton::clicked, this, &ProfileEditor::choosePhoto);
    connect(removePhotoButton, &QPushButton::clicked, this, &ProfileEditor::removePhoto);
    connect(saveButton, &QPushButton::clicked, this, &ProfileEditor::save);
}

void ProfileEditor::setProfile(const Profile &profile)
{
    m_profile = profile;
    m_savedPhotoPath = profile.photoPath;
    m_name->setText(profile.name);
    m_age->setValue(profile.age);
    m_gender->setCurrentIndex(m_gender->findData(int(profile.gender)));
    m_seeking->setCurrentIndex(m_seeking->findData(int(profile.seeking)));
    m_minAge->setValue(profile.minAge);
    m_maxAge->setValue(profile.maxAge);
    m_city->setCurrentIndex(std::max(0, m_city->findData(profile.city)));
    for (QCheckBox *box : m_interestBoxes)
        box->setChecked(profile.interests.contains(box->text()));
    m_bio->setPlainText(profile.bio);
    m_message->hide();
    updatePhotoPreview();
}

void ProfileEditor::updatePhotoPreview()
{
    m_profile.name = m_name->text().trimmed();
    m_photoPreview->setPixmap(avatarPixmap(m_profile, kPreviewSize, devicePixelRatioF()));
}

void ProfileEditor::choosePhoto()
{
    const QString source = QFileDialog::getOpenFileName(this, "Fotoğraf seç", QDir::homePath(),
                                                        "Resimler (*.png *.jpg *.jpeg)");
    if (source.isEmpty())
        return;

    // Dosya uzantısına değil içeriğe bakılır; boyut sınırları bozuk ya da dev dosyaları engeller
    if (QFileInfo(source).size() > kMaxPhotoBytes) {
        showMessage(m_message, "Fotoğraf en fazla 5 MB olabilir.", true);
        return;
    }
    QImageReader reader(source);
    const QByteArray format = reader.format();
    const QSize dimensions = reader.size();
    if ((format != "png" && format != "jpeg") || dimensions.isEmpty()
        || dimensions.width() > kMaxPhotoDimension || dimensions.height() > kMaxPhotoDimension) {
        showMessage(m_message, "Bu dosya geçerli bir PNG ya da JPG fotoğrafı değil.", true);
        return;
    }
    reader.setAutoTransform(true); // telefon fotoğraflarının yönünü düzeltir
    const QImage image = reader.read();
    if (image.isNull()) {
        showMessage(m_message, "Fotoğraf okunamadı.", true);
        return;
    }

    // Küçültülmüş bir kopya uygulamanın kendi klasörüne kaydedilir; asıl dosyaya dokunulmaz
    const QString target = QDir(m_photosDir).filePath(m_profile.id + ".png");
    const QImage scaled = image.scaled(kStoredPhotoSize, kStoredPhotoSize, Qt::KeepAspectRatioByExpanding,
                                       Qt::SmoothTransformation);
    if (!QDir().mkpath(m_photosDir) || !scaled.save(target, "PNG")) {
        showMessage(m_message, "Fotoğraf kaydedilemedi.", true);
        return;
    }

    m_profile.photoPath = target;
    updatePhotoPreview();
    showMessage(m_message, "Fotoğraf eklendi. Kalıcı olması için Kaydet'e bas.", false);
}

void ProfileEditor::removePhoto()
{
    m_profile.photoPath.clear();
    updatePhotoPreview();
    showMessage(m_message, "Fotoğraf kaldırıldı. Kalıcı olması için Kaydet'e bas.", false);
}

void ProfileEditor::save()
{
    QStringList interests;
    for (QCheckBox *box : m_interestBoxes) {
        if (box->isChecked())
            interests.append(box->text());
    }

    QStringList problems;
    if (m_name->text().trimmed().isEmpty())
        problems << "Adın boş olamaz.";
    if (m_minAge->value() > m_maxAge->value())
        problems << "Yaş aralığının başlangıcı bitişinden büyük olamaz.";
    if (interests.size() > kMaxInterests)
        problems << QString("En fazla %1 ilgi alanı seçebilirsin.").arg(kMaxInterests);
    if (m_bio->toPlainText().size() > kMaxBioLength)
        problems << QString("Hakkında yazısı en fazla %1 karakter olabilir.").arg(kMaxBioLength);
    if (!problems.isEmpty()) {
        showMessage(m_message, problems.join('\n'), true);
        return;
    }

    Profile updated = m_profile;
    updated.name = m_name->text().trimmed();
    updated.age = m_age->value();
    updated.gender = static_cast<Gender>(m_gender->currentData().toInt());
    updated.seeking = static_cast<Seeking>(m_seeking->currentData().toInt());
    updated.minAge = m_minAge->value();
    updated.maxAge = m_maxAge->value();
    updated.city = m_city->currentData().toString();
    updated.interests = interests;
    updated.bio = m_bio->toPlainText().trimmed();

    if (!m_db.saveProfile(updated)) {
        showMessage(m_message, "Profil kaydedilemedi. Lütfen tekrar dene.", true);
        return;
    }
    // Kaldırılan fotoğrafın dosyası da silinir (sadece uygulamanın kendi fotoğraf klasöründeyse)
    const bool photoRemoved = !m_savedPhotoPath.isEmpty() && m_savedPhotoPath != updated.photoPath;
    if (photoRemoved && QFileInfo(m_savedPhotoPath).absolutePath() == QFileInfo(m_photosDir).absoluteFilePath())
        QFile::remove(m_savedPhotoPath);
    m_savedPhotoPath = updated.photoPath;

    m_profile = updated;
    showMessage(m_message, "Profilin kaydedildi.", false);
    emit profileSaved(updated);
}
