#include "TimelineWidget.h"

#include <QPainter>
#include <QPixmap>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QtMath>
#include <algorithm>
#include <cmath>

TimelineWidget::TimelineWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(260);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);
}

void TimelineWidget::invalidate()
{
    cacheDirty_ = true;
    update();
}

void TimelineWidget::clear()
{
    events_.clear();
    laneIndex_.clear();
    laneNames_.clear();
    laneEvents_.clear();
    hoverIndex_ = -1;
    tMin_ = 0;
    tMax_ = 1;
    viewStart_ = 0;
    viewSpan_ = 1;
    invalidate();
}

void TimelineWidget::setEvents(const QVector<TimelineEvent> &events)
{
    events_ = events;
    hoverIndex_ = -1;
    rebuildLanes();

    if (events_.isEmpty()) {
        clear();
        return;
    }

    tMin_ = events_.first().timestamp;
    tMax_ = events_.first().timestamp;
    for (const auto &e : events_) {
        tMin_ = std::min<uint64_t>(tMin_, e.timestamp);
        tMax_ = std::max<uint64_t>(tMax_, e.timestamp);
    }
    if (tMax_ <= tMin_)
        tMax_ = tMin_ + 1;

    viewStart_ = double(tMin_);
    viewSpan_ = double(tMax_ - tMin_);
    clampView();
    invalidate();
}

void TimelineWidget::rebuildLanes()
{
    laneIndex_.clear();
    laneNames_.clear();
    laneEvents_.clear();

    for (int i = 0; i < events_.size(); ++i) {
        const auto &e = events_[i];
        auto it = laneIndex_.find(e.threadPointer);
        int lane;
        if (it == laneIndex_.end()) {
            lane = laneNames_.size();
            laneIndex_.insert(e.threadPointer, lane);
            laneNames_.append(
                e.threadName.isEmpty()
                    ? QString("0x%1").arg(e.threadPointer, 0, 16)
                    : e.threadName);
            laneEvents_.append(QVector<int>());
        } else {
            lane = it.value();
        }
        laneEvents_[lane].append(i);
    }

    for (auto &v : laneEvents_) {
        std::stable_sort(v.begin(), v.end(), [this](int a, int b) {
            return events_[a].timestamp < events_[b].timestamp;
        });
    }
}

void TimelineWidget::clampView()
{
    const double full = std::max(1.0, double(tMax_ - tMin_));
    /* allow zoom until ~2 time units visible (or full range) */
    const double minSpan = std::min(2.0, full);
    const double maxSpan = full;

    viewSpan_ = std::clamp(viewSpan_, minSpan, maxSpan);
    if (viewStart_ < double(tMin_))
        viewStart_ = double(tMin_);
    if (viewStart_ + viewSpan_ > double(tMax_))
        viewStart_ = double(tMax_) - viewSpan_;
    if (viewStart_ < double(tMin_))
        viewStart_ = double(tMin_);
}

double TimelineWidget::timeToX(uint64_t t) const
{
    const int plotW = std::max(1, width() - leftMargin_ - 16);
    return leftMargin_ + ((double(t) - viewStart_) / viewSpan_) * plotW;
}

uint64_t TimelineWidget::xToTime(double x) const
{
    const int plotW = std::max(1, width() - leftMargin_ - 16);
    const double t = viewStart_ + ((x - leftMargin_) / plotW) * viewSpan_;
    return uint64_t(std::llround(std::max(0.0, t)));
}

void TimelineWidget::drawTimeAxis(QPainter &p, const QRect &axisRect)
{
    p.fillRect(axisRect, palette().base());
    p.setPen(QPen(palette().mid().color(), 1));
    p.drawLine(axisRect.bottomLeft(), axisRect.bottomRight());

    const int plotW = axisRect.width();
    /* ~8–12 ticks */
    const int targetTicks = std::max(4, plotW / 80);
    double rawStep = viewSpan_ / targetTicks;
    const double pow10 = std::pow(10.0, std::floor(std::log10(std::max(rawStep, 1.0))));
    const double candidates[] = {1.0, 2.0, 5.0, 10.0};
    double step = pow10;
    for (double c : candidates) {
        if (c * pow10 >= rawStep) {
            step = c * pow10;
            break;
        }
    }

    const double first = std::ceil(viewStart_ / step) * step;
    p.setPen(palette().text().color());
    QFont f = font();
    f.setPointSizeF(f.pointSizeF() * 0.9);
    p.setFont(f);

    for (double t = first; t <= viewStart_ + viewSpan_ + step * 0.5; t += step) {
        const int x = int(timeToX(uint64_t(t)));
        if (x < axisRect.left() || x > axisRect.right())
            continue;
        p.setPen(QPen(palette().mid().color(), 1));
        p.drawLine(x, axisRect.bottom() - 4, x, axisRect.bottom());
        p.setPen(palette().text().color());
        const QString label = QString::number(qulonglong(t));
        p.drawText(QRect(x - 40, axisRect.top(), 80, axisRect.height() - 6),
                   Qt::AlignHCenter | Qt::AlignBottom, label);
    }

    p.setPen(palette().windowText().color());
    p.drawText(axisRect.adjusted(4, 0, -4, 0),
               Qt::AlignLeft | Qt::AlignTop,
               tr("time (ticks)"));
}

void TimelineWidget::drawLanes(QPainter &p, const QRect &plotRect)
{
    for (int i = 0; i < laneNames_.size(); ++i) {
        const int y = plotRect.top() + i * laneHeight_;
        const QRect laneBg(plotRect.left(), y, plotRect.width(), laneHeight_);
        if (i % 2)
            p.fillRect(laneBg, palette().alternateBase());

        p.setPen(palette().text().color());
        p.drawText(QRect(8, y, leftMargin_ - 16, laneHeight_),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   QFontMetrics(font()).elidedText(laneNames_[i], Qt::ElideRight, leftMargin_ - 20));

        p.setPen(QPen(palette().mid().color(), 1, Qt::DotLine));
        p.drawLine(plotRect.left(), y + laneHeight_ / 2,
                   plotRect.right(), y + laneHeight_ / 2);
    }
}

/* Cost is O(lanes * plotWidth / minEventPx_ * log N), independent of event count:
 * after painting an event, we binary-search past every event that would land
 * on the pixels just painted. */
void TimelineWidget::drawEvents(QPainter &p, const QRect &plotRect)
{
    p.save();
    p.setClipRect(plotRect);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(Qt::NoPen);

    const double plotW = std::max(1, width() - leftMargin_ - 16);
    const uint64_t tBegin = uint64_t(std::max(0.0, std::floor(viewStart_)));
    const uint64_t tEnd   = uint64_t(std::ceil(viewStart_ + viewSpan_));
    const int w = std::max(1, minEventPx_);

    auto tsLess = [this](int idx, uint64_t t) { return events_[idx].timestamp < t; };

    for (int lane = 0; lane < laneEvents_.size(); ++lane) {
        const QVector<int> &idxs = laneEvents_[lane];
        const int y = plotRect.top() + lane * laneHeight_ + 6;
        const int h = laneHeight_ - 12;

        auto it = std::lower_bound(idxs.begin(), idxs.end(), tBegin, tsLess);
        while (it != idxs.end()) {
            const TimelineEvent &e = events_[*it];
            if (e.timestamp > tEnd)
                break;

            const int x = int(timeToX(e.timestamp));
            p.fillRect(x - w / 2, y, w, h,
                       QColor::fromHsv((int(e.eventId) * 47) % 360, 160, 210));

            /* first time value that maps to a pixel right of what we just painted */
            const double tNextD = viewStart_ + ((x - w / 2 + w - leftMargin_) / plotW) * viewSpan_;
            const uint64_t tNext = uint64_t(std::max(0.0, std::ceil(tNextD)));
            it = std::lower_bound(it + 1, idxs.end(), tNext, tsLess);
        }
    }
    p.restore();
}

void TimelineWidget::renderScene(QPainter &p)
{
    p.fillRect(rect(), palette().window());

    const int plotTop = topMargin_ + axisHeight_;
    const int plotH = std::max(laneHeight_, int(laneNames_.size()) * laneHeight_);
    const QRect axisRect(leftMargin_, topMargin_, width() - leftMargin_ - 16, axisHeight_);
    const QRect plotRect(leftMargin_, plotTop, width() - leftMargin_ - 16, plotH);

    drawTimeAxis(p, axisRect);
    drawLanes(p, plotRect);
    drawEvents(p, plotRect);

    /* separator between labels and plot */
    p.setPen(QPen(palette().mid().color(), 1));
    p.drawLine(leftMargin_ - 1, topMargin_, leftMargin_ - 1, plotTop + plotH);

    p.setPen(palette().text().color());
    p.drawText(8, height() - 6,
               tr("Wheel: zoom to cursor   |   Drag: pan   |   Hover: details"));
}

void TimelineWidget::drawHover(QPainter &p)
{
    if (hoverIndex_ < 0 || hoverIndex_ >= events_.size())
        return;

    const auto &e = events_[hoverIndex_];
    const int plotTop = topMargin_ + axisHeight_;
    const int lane = laneIndex_.value(e.threadPointer, 0);
    const double x = timeToX(e.timestamp);
    const int y = plotTop + lane * laneHeight_ + 6;
    const int h = laneHeight_ - 12;

    p.save();
    p.setClipRect(QRect(leftMargin_, plotTop, width() - leftMargin_ - 16,
                        std::max(laneHeight_, int(laneNames_.size()) * laneHeight_)));
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(palette().highlight().color(), 2));
    p.setBrush(QColor::fromHsv((int(e.eventId) * 47) % 360, 160, 210).lighter(140));
    p.drawRoundedRect(QRectF(x - minEventPx_ / 2.0, y, minEventPx_, h), 2, 2);
    p.restore();
}

void TimelineWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (events_.isEmpty()) {
        p.fillRect(rect(), palette().window());
        p.setPen(palette().text().color());
        p.drawText(rect(), Qt::AlignCenter, tr("No events — open a trace"));
        return;
    }

    const qreal dpr = devicePixelRatioF();
    const QSize pxSize = (size() * dpr).expandedTo(QSize(1, 1));
    if (cacheDirty_ || cache_.size() != pxSize) {
        cache_ = QPixmap(pxSize);
        cache_.setDevicePixelRatio(dpr);
        QPainter cp(&cache_);
        renderScene(cp);
        cp.end();
        cacheDirty_ = false;
    }

    p.drawPixmap(0, 0, cache_);
    drawHover(p);
}

int TimelineWidget::hitTest(const QPoint &pos) const
{
    if (events_.isEmpty())
        return -1;

    const int plotTop = topMargin_ + axisHeight_;
    if (pos.y() < plotTop)
        return -1;
    const int lane = (pos.y() - plotTop) / laneHeight_;
    if (lane >= laneEvents_.size())
        return -1;

    const double plotW = std::max(1, width() - leftMargin_ - 16);
    auto tAt = [&](double x) { return viewStart_ + ((x - leftMargin_) / plotW) * viewSpan_; };

    const double lo = std::max(0.0, tAt(pos.x() - minEventPx_));
    const double hi = tAt(pos.x() + minEventPx_);
    if (hi < 0)
        return -1;

    const QVector<int> &idxs = laneEvents_[lane];
    auto it = std::lower_bound(idxs.begin(), idxs.end(), uint64_t(std::floor(lo)),
                               [this](int idx, uint64_t t) { return events_[idx].timestamp < t; });
    if (it == idxs.end() || double(events_[*it].timestamp) > hi)
        return -1;
    return *it;
}

void TimelineWidget::wheelEvent(QWheelEvent *event)
{
    if (events_.isEmpty())
        return;

    const QPointF pos = event->position();
    const double mouseT = double(xToTime(pos.x()));
    const double factor = (event->angleDelta().y() > 0) ? (1.0 / 1.2) : 1.2;

    viewSpan_ *= factor;
    /* keep time under cursor stable */
    const int plotW = std::max(1, width() - leftMargin_ - 16);
    const double frac = std::clamp((pos.x() - leftMargin_) / plotW, 0.0, 1.0);
    viewStart_ = mouseT - frac * viewSpan_;
    clampView();
    hoverIndex_ = -1;
    QToolTip::hideText();
    invalidate();
    event->accept();
}

void TimelineWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        panning_ = true;
        panLast_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void TimelineWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (panning_) {
        const int plotW = std::max(1, width() - leftMargin_ - 16);
        const double dx = event->pos().x() - panLast_.x();
        viewStart_ -= (dx / plotW) * viewSpan_;
        panLast_ = event->pos();
        clampView();
        hoverIndex_ = -1;
        QToolTip::hideText();
        invalidate();
        return;
    }

    const int hit = hitTest(event->pos());
    if (hit != hoverIndex_) {
        hoverIndex_ = hit;
        if (hit >= 0) {
            const auto &e = events_[hit];
            const QString text =
                tr("%1\nID: %2\nThread: %3\nTime: %4")
                    .arg(e.eventName.isEmpty() ? tr("event") : e.eventName)
                    .arg(e.eventId)
                    .arg(e.threadName.isEmpty()
                             ? QString("0x%1").arg(e.threadPointer, 0, 16)
                             : e.threadName)
                    .arg(qulonglong(e.timestamp));
            QToolTip::showText(event->globalPosition().toPoint(), text, this);
        } else {
            QToolTip::hideText();
        }
        update(); /* cheap: blits cached pixmap + one highlight */
    }
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        panning_ = false;
        setCursor(Qt::ArrowCursor);
    }
}

void TimelineWidget::leaveEvent(QEvent *)
{
    hoverIndex_ = -1;
    QToolTip::hideText();
    update();
}

void TimelineWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    clampView();
    invalidate();
}