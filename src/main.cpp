#include <QApplication>
#include <QLabel>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("DatingApp");
    QApplication::setApplicationVersion(APP_VERSION);

    QMainWindow window;
    window.setWindowTitle("Dating App");
    window.resize(1000, 700);
    window.setCentralWidget(new QLabel(QString("Merhaba Qt! Sürüm %1").arg(APP_VERSION)));
    window.show();

    return app.exec();
}
