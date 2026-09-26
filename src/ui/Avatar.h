#pragma once

#include "core/Profile.h"

#include <QPixmap>

// Profilin yuvarlak avatarı: fotoğrafı varsa kırpılmış fotoğraf, yoksa baş harfli renkli daire.
// devicePixelRatio: ekran ölçeklemesi (%125, %150...) için keskin çizim
QPixmap avatarPixmap(const Profile &profile, int size, qreal devicePixelRatio = 1.0);
