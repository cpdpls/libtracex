#include "StatsResultWidget.h"
#include <QtMath>
#include <QPaintEvent>

ChartCanvas::ChartCanvas(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(220);
}

void ChartCanvas::setData(Mode mode, const QStringList &labels, const QVector<double> &values)
{
    mode_ = mode;
    labels_ = labels;
    values_ = values;
    update();
}

void ChartCanvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), palette().base());
    p.setRenderHint(QPainter::Antialiasing, true);

    if (values_.isEmpty()) {
        p.drawText(rect(), Qt::AlignCenter, tr("No data"));
        return;
    }

    double sum = 0, maxV = 0;
    for (double v : values_) {
        sum += std::max(0.0, v);
        maxV = std::max(maxV, v);
    }
    if (maxV <= 0) maxV = 1;
    if (sum <= 0) sum = 1;

    if (mode_ == Bar) {
        const int n = values_.size();
        const int left = 50, right = 16, top = 16, bottom = 40;
        const int w = width() - left - right;
        const int h = height() - top - bottom;
        const double barW = std::max(4.0, double(w) / std::max(1, n) - 4.0);

        for (int i = 0; i < n; ++i) {
            const double bh = (values_[i] / maxV) * h;
            const double x = left + i * (barW + 4);
            const double y = top + h - bh;
            QColor c = QColor::fromHsv((i * 53) % 360, 180, 220);
            p.fillRect(QRectF(x, y, barW, bh), c);
            p.setPen(palette().text().color());
            p.save();
            p.translate(x + barW / 2, height() - 8);
            p.rotate(-35);
            p.drawText(QRect(-40, -16, 80, 16), Qt::AlignCenter,
                       QFontMetrics(font()).elidedText(
                           i < labels_.size() ? labels_[i] : QString::number(i),
                           Qt::ElideRight, 70));
            p.restore();
        }
        p.setPen(palette().mid().color());
        p.drawLine(left, top + h, left + w, top + h);
        p.drawLine(left, top, left, top + h);
    } else { /* Pie */
        QRectF pie = rect().adjusted(20, 20, -20, -40);
        const double side = std::min(pie.width(), pie.height());
        pie = QRectF(pie.center().x() - side / 2, pie.center().y() - side / 2, side, side);

        double angle = 90.0 * 16.0; /* Qt: 1/16 degree, start at top */
        for (int i = 0; i < values_.size(); ++i) {
            const double span = (std::max(0.0, values_[i]) / sum) * 360.0 * 16.0;
            QColor c = QColor::fromHsv((i * 53) % 360, 180, 220);
            p.setBrush(c);
            p.setPen(QPen(palette().base().color(), 1));
            p.drawPie(pie, int(angle), int(-span));
            angle -= span;
        }

        /* legend */
        int ly = 8;
        for (int i = 0; i < values_.size() && i < 12; ++i) {
            QColor c = QColor::fromHsv((i * 53) % 360, 180, 220);
            p.fillRect(8, ly, 12, 12, c);
            p.setPen(palette().text().color());
            const QString name = i < labels_.size() ? labels_[i] : QString::number(i);
            p.drawText(24, ly + 11,
                       QStringLiteral("%1 (%2)")
                           .arg(name)
                           .arg(values_[i], 0, 'f', 1));
            ly += 16;
        }
    }
}

StatsResultWidget::StatsResultWidget(QWidget *parent) : QWidget(parent)
{
    title_ = new QLabel(this);
    desc_ = new QLabel(this);
    viewHint_ = new QLabel(this);
    scriptHint_ = new QLabel(this);
    table_ = new QTableWidget(this);
    chart_ = new ChartCanvas(this);

    title_->setStyleSheet(QStringLiteral("font-weight:bold; font-size:13pt;"));
    desc_->setWordWrap(true);
    scriptHint_->setStyleSheet(QStringLiteral("color: gray;"));
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setAlternatingRowColors(true);
    table_->horizontalHeader()->setStretchLastSection(true);

    auto *lay = new QVBoxLayout(this);
    lay->addWidget(title_);
    lay->addWidget(desc_);
    lay->addWidget(viewHint_);
    lay->addWidget(scriptHint_);
    lay->addWidget(table_, 1);
    lay->addWidget(chart_, 1);
    chart_->hide();
}

void StatsResultWidget::setScriptPath(const QString &path)
{
    scriptPath_ = path;
    scriptHint_->setText(tr("Script: %1").arg(path));
}

int StatsResultWidget::findLabelColumn(const tracex_stats_results *res)
{
    for (size_t c = 0; c < res->n_columns; ++c)
        if (res->columns[c].type == COL_STRING)
            return int(c);
    return res->n_columns ? 0 : -1;
}

int StatsResultWidget::findValueColumn(const tracex_stats_results *res)
{
    /* prefer usage_pct / percent / cpu_percent style doubles, else first numeric */
    for (size_t c = 0; c < res->n_columns; ++c) {
        const QString n = QString::fromUtf8(res->columns[c].name).toLower();
        if (res->columns[c].type == COL_DOUBLE &&
            (n.contains(QStringLiteral("pct")) || n.contains(QStringLiteral("percent"))))
            return int(c);
    }
    for (size_t c = 0; c < res->n_columns; ++c) {
        if (res->columns[c].type == COL_DOUBLE ||
            res->columns[c].type == COL_UINT32 ||
            res->columns[c].type == COL_UINT64)
            return int(c);
    }
    return -1;
}

void StatsResultWidget::showTable(const tracex_stats_results *res)
{
    table_->show();
    chart_->hide();
    table_->setColumnCount(int(res->n_columns));
    table_->setRowCount(int(res->n_rows));

    QStringList headers;
    for (size_t c = 0; c < res->n_columns; ++c) {
        QString h = QString::fromUtf8(res->columns[c].name);
        if (res->columns[c].unit[0])
            h += QStringLiteral(" (%1)").arg(QString::fromUtf8(res->columns[c].unit));
        headers << h;
    }
    table_->setHorizontalHeaderLabels(headers);

    for (size_t r = 0; r < res->n_rows; ++r) {
        for (size_t c = 0; c < res->n_columns; ++c) {
            QString text;
            const auto &v = res->rows[r].values[c];
            switch (res->columns[c].type) {
            case COL_UINT32: text = QString::number(v.u32); break;
            case COL_UINT64: text = QString::number(qulonglong(v.u64)); break;
            case COL_DOUBLE: text = QString::number(v.f64, 'f', 2); break;
            case COL_STRING: text = v.str ? QString::fromUtf8(v.str) : QString(); break;
            case COL_BOOL:   text = v.b ? QStringLiteral("true") : QStringLiteral("false"); break;
            }
            table_->setItem(int(r), int(c), new QTableWidgetItem(text));
        }
    }
    table_->resizeColumnsToContents();
}

void StatsResultWidget::showChart(const tracex_stats_results *res, ChartCanvas::Mode mode)
{
    showTable(res);          // fills the table (and hides the chart)
    chart_->show();          // now reveal the chart

    const int labelCol = findLabelColumn(res);
    const int valueCol = findValueColumn(res);
    if (labelCol < 0 || valueCol < 0) {
        chart_->hide();      // nothing to plot
        return;
    }

    QStringList labels;
    QVector<double> values;
    for (size_t r = 0; r < res->n_rows; ++r) {
        const auto &lv = res->rows[r].values[labelCol];
        const auto &vv = res->rows[r].values[valueCol];
        if (res->columns[labelCol].type == COL_STRING)
            labels << (lv.str ? QString::fromUtf8(lv.str) : QString());
        else
            labels << QString::number(lv.u32);

        double x = 0;
        switch (res->columns[valueCol].type) {
        case COL_UINT32: x = vv.u32; break;
        case COL_UINT64: x = double(vv.u64); break;
        case COL_DOUBLE: x = vv.f64; break;
        default: break;
        }
        values.append(x);
    }
    chart_->setData(mode, labels, values);
}

void StatsResultWidget::setResults(const tracex_stats_results *res)
{
    table_->clear();
    table_->setRowCount(0);
    table_->setColumnCount(0);
    chart_->hide();

    if (!res) {
        title_->setText(tr("No statistics"));
        desc_->clear();
        viewHint_->clear();
        return;
    }

    title_->setText(QString::fromUtf8(res->name));
    desc_->setText(QString::fromUtf8(res->description));
    const QString view = QString::fromUtf8(res->suggested_view).toLower();
    viewHint_->setText(tr("Suggested view: %1").arg(view));

    if (view == QLatin1String("pie"))
        showChart(res, ChartCanvas::Pie);
    else if (view == QLatin1String("bar") ||
             view == QLatin1String("sticks") ||
             view == QLatin1String("timeline"))
        showChart(res, ChartCanvas::Bar);
    else
        showTable(res);
}