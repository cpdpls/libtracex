#pragma once

#include <QWidget>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QPixmap>
#include <QPoint>
#include <cstdint>

struct TimelineEvent {
    uint32_t eventId = 0;
    uint32_t threadPointer = 0;
    uint64_t timestamp = 0;
    QString  eventName;
    QString  threadName;
};

class TimelineWidget : public QWidget {
    Q_OBJECT
public:
    explicit TimelineWidget(QWidget *parent = nullptr);

    void setEvents(const QVector<TimelineEvent> &events);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void rebuildLanes();
    void clampView();
    void invalidate();                 /* mark cached scene dirty + repaint */
    int  hitTest(const QPoint &pos) const;
    double timeToX(uint64_t t) const;
    uint64_t xToTime(double x) const;
    void renderScene(QPainter &p);     /* full scene -> cache_ */
    void drawHover(QPainter &p);       /* cheap overlay, drawn every paint */
    void drawTimeAxis(QPainter &p, const QRect &axisRect);
    void drawLanes(QPainter &p, const QRect &plotRect);
    void drawEvents(QPainter &p, const QRect &plotRect);

    QVector<TimelineEvent> events_;
    QHash<uint32_t, int> laneIndex_;
    QStringList laneNames_;
    QVector<QVector<int>> laneEvents_;  /* per lane: event indices sorted by timestamp */

    QPixmap cache_;
    bool cacheDirty_ = true;

    uint64_t tMin_ = 0;
    uint64_t tMax_ = 1;
    /* visible window in time units */
    double viewStart_ = 0;
    double viewSpan_ = 1;

    int leftMargin_ = 150;
    int topMargin_ = 8;
    int axisHeight_ = 28;
    int laneHeight_ = 36;
    int minEventPx_ = 6;   /* never thinner than this when drawing */

    bool panning_ = false;
    QPoint panLast_;
    int hoverIndex_ = -1;
};