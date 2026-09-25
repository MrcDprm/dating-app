# Dating App

C++ ve Qt 6 ile yazılmış masaüstü eşleştirme uygulaması. Profilleri uyum puanına göre sıralar; eşleştiğin hazır profiller yapay zekâ ile kendi karakterlerine bürünerek sohbete cevap verir.

> Bu bir portfolyo projesidir: ağ üzerinden gerçek kullanıcılar yoktur. Hazır profillerin hepsi hayalidir ve sohbette yapay zekâ olduklarını belirten bir etiket taşır.

## Özellikler (plan)

**MVP**
- [ ] Kayıt ve giriş: şifreler Argon2id (libsodium) ile hash'lenir, zayıf şifre ve 18 yaş altı kabul edilmez
- [ ] Profil: ad, yaş, cinsiyet, aradığı cinsiyet, yaş aralığı, şehir, ilgi alanları, hakkında yazısı, fotoğraf
- [ ] Fotoğraf yükleme: tür (PNG/JPG), boyut ve içerik kontrolü; fotoğraf küçültülüp kullanıcı klasörüne kopyalanır
- [ ] Uyum puanı (0-100): ortak ilgi alanları, yaş uyumu, şehir; neden uyumlu olduğunuzu gösteren açıklama
- [ ] Keşfet ekranı: uyum puanına göre sıralı kartlar, beğen / geç
- [ ] Eşleşme: karşılıklı beğenide "Eşleşme!" bildirimi (hazır profiller kendi tercihlerine göre geri beğenir)
- [ ] Eşleşmelerim listesi ve eşleşmeyi kaldırma
- [ ] Mesajlaşma: eşleşilen profil yapay zekâ ile kendi karakterinde cevap verir, sohbet geçmişi saklanır
- [ ] Yapay zekâ sağlayıcısı: varsayılan yerel **Ollama** (ücretsiz, anahtar gerekmez); isteğe bağlı **Claude API** (kullanıcının kendi anahtarı, Windows DPAPI ile şifreli saklanır)
- [ ] Yapay zekâya ulaşılamazsa anlaşılır hata mesajı; uygulama çökmez
- [ ] 40 hazır örnek profil (hayali karakterler; fotoğrafı olmayan profillere baş harfli renkli avatar çizilir)
- [ ] Veriler SQLite ile kullanıcı klasöründe (`%APPDATA%\DatingApp`)
- [ ] Ayarlar, Hakkında penceresi, uygulama ikonu, sürüm numarası
- [ ] Birim testleri (uyum puanı, sıralama, şifre kontrolü)
- [ ] Windows kurulum dosyası (windeployqt + Inno Setup)

**Sonra eklenebilecekler**
- Filtreler (şehir, yaş aralığı) ve arama
- Koyu / açık tema
- Mesajlarda "yazıyor…" animasyonu ve akışlı (streaming) cevap

## Mimari (plan)

```
src/
├── core/   Uyum puanı, sıralama, şifre hash'leme (arayüzden bağımsız, test edilebilir)
├── data/   SQLite veritabanı, hazır profillerin yüklenmesi
├── ai/     Sohbet sağlayıcıları (Ollama, Claude) ve karakter talimatı
└── ui/     Giriş, Keşfet, Eşleşmeler, Sohbet, Profil, Ayarlar, Hakkında
resources/  İkon, avatarlar, hazır profiller (JSON)
tests/      Qt Test birim testleri
installer/  Inno Setup betiği
```

## Kullanılan Teknolojiler

- C++20, CMake
- Qt 6 (Widgets, Sql, Network)
- SQLite
- libsodium (Argon2id)
- Ollama / Claude API

## Kurulum ve Çalıştırma

_Yazılacak._

## Öğrendiklerim

_Yazılacak._

## Gelecek Planları

_Yazılacak._
