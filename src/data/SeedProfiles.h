#pragma once

#include <QByteArray>

class Database;

// Hazır (yapay zekâ) profilleri JSON'dan okuyup veritabanına kaydeder.
// Kaydedilen profil sayısını döndürür; JSON okunamazsa -1 döner.
int importSeedProfiles(Database &db, const QByteArray &json);
