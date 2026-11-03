#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString filePath;
    QString scriptPath;

    for (int i = 1; i < argc; ++i) {
        const QString a = QString::fromLocal8Bit(argv[i]);
        if ((a == QLatin1String("-f") || a == QLatin1String("--file")) && i + 1 < argc)
            filePath = QString::fromLocal8Bit(argv[++i]);
        else if ((a == QLatin1String("-s") || a == QLatin1String("--script")) && i + 1 < argc)
            scriptPath = QString::fromLocal8Bit(argv[++i]);
    }

    MainWindow w;
    if (!scriptPath.isEmpty())
        w.setInitialScript(scriptPath);
    if (!filePath.isEmpty()) {
        w.setInitialTrace(filePath);
        /* open UI first, then run */
        w.show();
        QMetaObject::invokeMethod(&w, "runAnalysis", Qt::QueuedConnection);
    } else {
        w.show();
    }

    return app.exec();
}