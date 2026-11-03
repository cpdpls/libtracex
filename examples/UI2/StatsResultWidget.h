#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPainter>
#include <QVector>

extern "C" {
#include "tracex_stats/tracex_stats.h"
}

class ChartCanvas : public QWidget {
    Q_OBJECT
public:
    enum Mode { Bar, Pie };
    explicit ChartCanvas(QWidget *parent = nullptr);
    void setData(Mode mode, const QStringList &labels, const QVector<double> &values);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Mode mode_ = Bar;
    QStringList labels_;
    QVector<double> values_;
};

class StatsResultWidget : public QWidget {
    Q_OBJECT
public:
    explicit StatsResultWidget(QWidget *parent = nullptr);
    void setResults(const tracex_stats_results *res);
    void setScriptPath(const QString &path);
    QString scriptPath() const { return scriptPath_; }

private:
    void showTable(const tracex_stats_results *res);
    void showChart(const tracex_stats_results *res, ChartCanvas::Mode mode);
    static int findLabelColumn(const tracex_stats_results *res);
    static int findValueColumn(const tracex_stats_results *res);

    QString scriptPath_;
    QLabel *title_;
    QLabel *desc_;
    QLabel *viewHint_;
    QLabel *scriptHint_;
    QTableWidget *table_;
    ChartCanvas *chart_;
};