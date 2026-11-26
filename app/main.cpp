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
#include <QQuickWindow>
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
    cli.addOption({"grab", "Save what the window shows as <prefix>-NN.png at each --grab-at time, "
                           "then quit (for scripts/verify).", "prefix"});
    cli.addOption({"grab-at", "Comma-separated milliseconds after start to grab (default 1500).", "ms",
                   "1500"});
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
    // In grab mode the window logs each slide it shows, for scripts/verify's ledger.
    engine.rootContext()->setContextProperty("logSlides", cli.isSet("grab"));
    engine.rootContext()->setContextProperty("intervalMs", qMax(5, cli.value("interval").toInt()) * 1000);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.load(QUrl("qrc:/Screensaver.qml"));

    if (cli.isSet("grab")) {
        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
        if (!window) return 1;
        const QString prefix = cli.value("grab");
        const QStringList times = cli.value("grab-at").split(',', Qt::SkipEmptyParts);
        int last = 0;
        for (int i = 0; i < times.size(); ++i) {
            const int ms = times[i].toInt();
            last = qMax(last, ms);
            QTimer::singleShot(ms, &app, [window, prefix, i] {
                const QString file = QString("%1-%2.png").arg(prefix).arg(i, 2, 10, QChar('0'));
                if (!window->grabWindow().save(file)) qCritical("Could not save '%s'.", qUtf8Printable(file));
            });
        }
        QTimer::singleShot(last + 200, &app, [] { QCoreApplication::quit(); });
    }

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
