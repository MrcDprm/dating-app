#pragma once

#include <QDialog>

// Hakkında penceresi: sürüm, açıklama, lisans ve kullanılan kütüphaneler
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
};
