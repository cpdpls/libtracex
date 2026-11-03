#pragma once

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QList>
#include <QSplitter>
#include <QVector>
#include <QHash>

#include "StatsResultWidget.h"
#include "TimelineWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void setInitialTrace(const QString &path);
    void setInitialScript(const QString &path);

private slots:
    void openTrace();
    void addStatisticScript();
    void clearStatisticScripts();
    void runAnalysis();

private:
    bool parseAndAnalyze();
    void ensureStatsPaneCount(int n);

    QString tracePath_;
    QStringList scriptPaths_;

    TimelineWidget *timelineWidget_;
    QSplitter *statsSplitter_;
    QList<StatsResultWidget *> statsViews_;
};