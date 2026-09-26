<p align="center">
  <img src="resources/icon.png" alt="Dating App ikonu" width="96">
</p>

<h1 align="center">Dating App</h1>

<p align="center">
  <a href="README.md">English</a> | <b>Türkçe</b>
</p>

<p align="center">
  C++ ve Qt 6 ile yazılmış masaüstü tanışma uygulaması. Profilleri uyum puanına göre sıralar;<br>
  eşleştiğin karakterler yapay zekâ ile kendi kişiliklerine bürünerek sohbete cevap verir.
</p>

<p align="center">
  <a href="https://github.com/MrcDprm/dating-app/releases/latest"><b>⬇️ Windows için indir</b></a>
</p>

<p align="center">
  <img src="docs/chat.png" alt="Eşleşilen karakterle yapay zekâ destekli sohbet" width="720">
</p>

> **Not:** Bu bir portfolyo projesidir. Ağ üzerinden gerçek kullanıcılar yoktur; 100 hazır profilin hepsi hayalidir
> ve sohbet cevapları yapay zekâ tarafından üretilir. Uygulama bunu her sohbette açıkça belirtir.

## Özellikler

**Eşleştirme**
- 0-100 arası **uyum puanı**: ortak ilgi alanları (60), yaş yakınlığı (25) ve aynı şehir (15)
- İki yönlü uygunluk kontrolü: iki kişi de birbirinin aradığı cinsiyete ve yaş aralığına uymalı
- Keşfet ekranı: adaylar uyum puanına göre sıralı, her kartta "neden uyumlusunuz" açıklaması
- Beğen / geç (düğmeler ya da ← → ok tuşları), karşılıklı beğenide "Eşleşme!" bildirimi
- Hazır profiller uyum puanına bağlı bir olasılıkla seni geri beğenir
- Geçilen profilleri tekrar gösterme, eşleşmeyi kaldırma

**Yapay zekâ ile sohbet**
- Her hazır profilin kendi kişiliği ve konuşma tarzı var; yapay zekâ bu karakterle cevap verir
- **Ollama** (varsayılan): bilgisayarında çalışır, ücretsizdir, anahtar gerekmez
- **Claude API** (isteğe bağlı): kendi anahtarınla daha iyi Türkçe
- "… yazıyor" göstergesi, hata olursa anlaşılır mesaj ve **Tekrar dene** düğmesi
- Sohbet geçmişi saklanır, karakter önceki konuşmayı hatırlar

**Profil ve hesap**
- Kayıt ve giriş; güçlü şifre kuralları, 5 hatalı denemede 30 saniye bekleme
- Profil: ad, yaş, cinsiyet, kimi aradığın, yaş aralığı, şehir (81 il), ilgi alanları, hakkında yazısı
- Profil fotoğrafı; fotoğrafı olmayan profillere baş harfli renkli avatar
- Birden fazla hesap, çıkış yap / tekrar giriş

**Güvenlik**
- Şifreler **Argon2id** (libsodium) ile hash'lenir, düz metin saklanmaz
- Claude API anahtarı **Windows DPAPI** ile şifrelenir ve sadece `api.anthropic.com` adresine gönderilir
- Tüm SQL sorguları parametreli; dosyadan ve yapay zekâdan gelen veriler doğrulanır
- Fotoğraf yüklemede tür, boyut ve içerik kontrolü; kullanıcı metinleri hiçbir yerde HTML olarak yorumlanmaz
- Tahmin edilemeyen UUID kimlikler, 18 yaş sınırı

## Ekran Görüntüleri

| Keşfet | Profil |
|:---:|:---:|
| <img src="docs/discover.png" alt="Keşfet ekranı" width="420"> | <img src="docs/profile.png" alt="Profil düzenleme" width="420"> |

| Kayıt | Ayarlar |
|:---:|:---:|
| <img src="docs/register.png" alt="Kayıt penceresi" width="300"> | <img src="docs/settings.png" alt="Yapay zekâ ayarları" width="360"> |

## Kurulum

1. [Releases](https://github.com/MrcDprm/dating-app/releases/latest) sayfasından `DatingApp-1.0.0-Setup.exe` dosyasını indir ve çalıştır.
2. Sohbet için yapay zekâ gerekir. Ücretsiz seçenek **Ollama**:
   - [ollama.com](https://ollama.com) adresinden Ollama'yı kur.
   - Terminalde modeli indir (yaklaşık 4,7 GB):
     ```
     ollama pull qwen2.5:7b-instruct
     ```
   - Farklı bir model kullanmak istersen uygulamada **Hesap → Ayarlar**'dan model adını değiştirebilirsin.
3. İstersen Ollama yerine **Claude API** kullanabilirsin: **Hesap → Ayarlar**'dan Claude'u seçip kendi API anahtarını gir. Kullanım ücretlidir.

Veriler `%APPDATA%\DatingApp` klasöründe saklanır. Kaldırma işlemi Windows'un "Uygulamalar" ekranından yapılır.

## Kullanılan Teknolojiler

- **C++20**, **CMake**, **Ninja**
- **Qt 6**: Widgets (arayüz), Sql (SQLite), Network (HTTP), Test (birim testleri)
- **SQLite**: yerel veritabanı
- **libsodium**: Argon2id şifre hash'leme
- **Windows DPAPI**: API anahtarını şifreleme
- **Ollama** / **Claude API**: yapay zekâ ile sohbet
- **Inno Setup**: Windows kurulum dosyası

## Proje Yapısı

```
src/
├── core/        Profil modeli, uyum puanı, şifre kuralları ve hash'leme (arayüzden bağımsız)
├── data/        SQLite veritabanı, hazır profillerin yüklenmesi
├── ai/          Sohbet sağlayıcıları (Ollama, Claude), karakter talimatı, anahtar şifreleme, ayarlar
├── ui/          Giriş, Keşfet, Eşleşmeler, Sohbet, Profil, Ayarlar, Hakkında
└── main.cpp
resources/       İkon, sürüm bilgisi şablonu, 100 hazır profil (JSON)
tests/           Qt Test birim testleri
installer/       Inno Setup betiği
```

## Kaynak Koddan Çalıştırma

[MSYS2](https://www.msys2.org) kurulu olmalı. **MSYS2 UCRT64** terminalinde gerekli paketleri kur:

```
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-qt6-base mingw-w64-ucrt-x86_64-qt6-tools mingw-w64-ucrt-x86_64-libsodium
```

`C:\msys64\ucrt64\bin` klasörünü PATH'e ekledikten sonra proje klasöründe:

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
.\build\DatingApp.exe
```

## Öğrendiklerim

- **C++'ta bellek ve sahiplik.** C#'ta çöp toplayıcı her şeyi hallediyordu; burada `new` ile oluşturduğum her nesnenin kimin olduğunu düşünmem gerekti. Qt'nin "üst nesne ölünce çocuklarını da siler" kuralını, referans (`&`) ile işaretçi (`*`) arasındaki farkı ve bir nesneyi kopyalamadan geçirmek için `const &` kullanmayı öğrendim.
- **Başlık ve kaynak dosyaları.** `.h` dosyasında "ne var", `.cpp` dosyasında "nasıl çalışıyor" yazılıyor. Bir fonksiyonu bildirip gövdesini yazmayı unutunca derleyicinin değil bağlayıcının (linker) hata verdiğini yaşayarak gördüm.
- **CMake ile proje kurmak.** Kaynak dosyaları, kütüphaneleri ve testleri tanımlamayı; çekirdek kodu ayrı bir kütüphaneye koyup hem uygulamaya hem testlere bağlamayı öğrendim. Bilgisayarda iki farklı derleyici kurulu olunca nasıl karıştığını ve derleme önbelleğini neden silmek gerektiğini de gördüm.
- **Qt ile masaüstü arayüzü.** Düzen nesneleri (layout), sekmeler, diyaloglar ve C#'taki event'lerin karşılığı olan sinyal/slot sistemini kullandım. Arayüzün cevap beklerken donmaması için ağ isteklerini asenkron yapmayı öğrendim.
- **Veritabanı tasarımı.** Profilleri, beğenileri ve mesajları SQLite'ta tuttum. Eşleşmeyi ayrı bir tabloda saklamak yerine "iki taraf da beğendiyse eşleşme vardır" diye sorguyla bulmayı, işlemleri transaction ile birlikte geri alınabilir yapmayı ve her sorguda parametre kullanmayı öğrendim.
- **Güvenliği baştan düşünmek.** Şifreleri Argon2id ile hash'lemeyi, API anahtarını Windows'un kendi şifrelemesiyle saklamayı, dosyadan ya da yapay zekâdan gelen hiçbir veriye körü körüne güvenmemeyi ve kullanıcı metinlerinin HTML olarak yorumlanmasını engellemeyi öğrendim.
- **Yapay zekâyı bir uygulamaya bağlamak.** HTTP üzerinden yerel bir modele (Ollama) ve bulut API'sine (Claude) istek göndermeyi, ikisini ortak bir arayüzün arkasına koyup ayarlardan değiştirilebilir yapmayı öğrendim. Küçük modellerin talimata nasıl daha iyi uyduğunu deneyerek gördüm: talimatı İngilizce yazıp Türkçe cevap istemek ve örnek bir konuşma eklemek sonucu belirgin şekilde iyileştirdi.
- **Test etmek.** Uyum puanı, şifre kuralları ve veritabanı için Qt Test ile birim testleri yazdım. Testlerde veritabanını bellekte açmak, her testin diğerinden bağımsız ve hızlı çalışmasını sağladı.

## Gelecek Planları

- Filtreler (şehir, yaş aralığı) ve arama
- Açık / koyu tema seçeneği
- Cevapların kelime kelime akarak gelmesi (streaming)
- Sohbette karşı tarafın da ilk mesajı atabilmesi
- İngilizce arayüz

## Lisans

[MIT](LICENSE) © 2026 Miraç Deprem. Uygulama [Qt 6](https://www.qt.io) (LGPLv3), SQLite ve libsodium kullanır.
