// Album-art screensaver: a fullscreen, shuffled slideshow of the images in one folder.
// Runs one instance per monitor (the launcher starts them). Any key, click,
// scroll or real mouse movement closes it, and so does focus leaving the
// screensaver windows, so dismissing one monitor's copy closes the others.
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QRandomGenerator>
#include <QTimer>
#include <QUrl>
#include <algorithm>

static const char *kAppId = "org.omarchy.screensaver";

// True while Hyprland's focused window is one of the screensaver windows.
static bool screensaverHasFocus() {
    QProcess p;
    p.start("hyprctl", {"activewindow", "-j"});
    if (!p.waitForFinished(1000)) return true;  // can't tell: keep running
    const auto obj = QJsonDocument::fromJson(p.readAllStandardOutput()).object();
    return obj.value("class").toString() == kAppId;
}

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    // The Wayland app id: Omarchy's window rules make this class fullscreen, and
    // its idle service watches for it to know the screensaver is up.
    QGuiApplication::setDesktopFileName(kAppId);
    QGuiApplication::setApplicationName("album-art-screensaver");

    QCommandLineParser cli;
    cli.setApplicationDescription("Fullscreen shuffled slideshow of album art.");
    cli.addHelpOption();
    cli.addOption({"dir", "Folder of images.", "path"});
    cli.addOption({"interval", "Seconds each image stays up (default 60).", "seconds", "60"});
    cli.addOption({"no-focus-exit", "Don't quit when focus leaves the screensaver (for testing)."});
    cli.process(app);

    const QString dir = cli.value("dir");
    QStringList files = QDir(dir).entryList(
        {"*.jpg", "*.jpeg", "*.png", "*.webp", "*.JPG", "*.JPEG", "*.PNG", "*.WEBP"},
        QDir::Files | QDir::Readable, QDir::Name);
    if (files.isEmpty()) {
        qCritical("No images found in '%s'.", qUtf8Printable(dir));
        return 2;
    }
    std::shuffle(files.begin(), files.end(), *QRandomGenerator::global());

    QStringList urls, captions;
    for (const QString &f : files) {
        urls << QUrl::fromLocalFile(QDir(dir).absoluteFilePath(f)).toString();
        captions << QFileInfo(f).completeBaseName();
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("imageUrls", urls);
    engine.rootContext()->setContextProperty("imageCaptions", captions);
    engine.rootContext()->setContextProperty("intervalMs", qMax(5, cli.value("interval").toInt()) * 1000);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.load(QUrl("qrc:/Screensaver.qml"));

    // Quit once focus has moved to a non-screensaver window (after a short
    // grace period while the launcher maps a window on each monitor).
    QTimer focusCheck;
    if (!cli.isSet("no-focus-exit")) {
        QObject::connect(&focusCheck, &QTimer::timeout, &app, [] {
            if (!screensaverHasFocus()) QCoreApplication::quit();
        });
        QTimer::singleShot(4000, &app, [&focusCheck] { focusCheck.start(1000); });
    }
    return app.exec();
}
