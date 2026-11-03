#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QFile>
#include <QByteArray>

extern "C" {
#include "tracex/tracex.h"
#include "labels_engine/labels_engine.h"
#include "tracex_stats/tracex_stats.h"
#include "tracex_stats/tracex_stats_errno.h"
}

/* ---- shared state for C callbacks ---- */
static tracex_handler_t *g_tx = nullptr;
static QVector<tracex_stats_handler_t *> *g_statsHandlers = nullptr;
static QVector<TimelineEvent> *g_timeline = nullptr;
static QHash<uint32_t, QString> *g_objNames = nullptr;

static void on_header(tracex_handler_t *, struct tracex_header *header, tracex_ret_t status)
{
    (void)header;
    (void)status;
}

static void on_object(tracex_handler_t *, struct tracex_object *object, tracex_ret_t status)
{
    if (status != TRACEX_SUCCESS || object == nullptr)
        return;

    struct tracex_stats_object o;
    o.type = object->type;
    o.pointer = object->pointer;
    o.name = object->name;
    o.param1 = object->params.param1;
    o.param2 = object->params.param2;

    if (g_statsHandlers) {
        for (int i = 0; i < g_statsHandlers->size(); ++i)
            tracex_stats_push_object(g_statsHandlers->at(i), &o);
    }

    if (g_objNames && object->name)
        g_objNames->insert(object->pointer,
            QString::fromUtf8(reinterpret_cast<const char *>(object->name)));
}

static void on_event(tracex_handler_t *, struct tracex_event *event, tracex_ret_t status)
{
    if (status != TRACEX_SUCCESS || event == nullptr)
        return;

    struct tracex_stats_event e;
    e.event_id = event->eventId;
    e.thread_pointer = event->threadPointer;
    e.thread_priority = event->threadPriority;
    e.i1 = event->rawInfos.info1;
    e.i2 = event->rawInfos.info2;
    e.i3 = event->rawInfos.info3;
    e.i4 = event->rawInfos.info4;
    e.timestamp = event->timeStamp;

    if (g_statsHandlers) {
        for (int i = 0; i < g_statsHandlers->size(); ++i)
            tracex_stats_push_event(g_statsHandlers->at(i), &e);
    }

    if (g_timeline) {
        TimelineEvent te;
        te.eventId = event->eventId;
        te.threadPointer = event->threadPointer;
        te.timestamp = event->timeStamp;

        if (event->labels.event_name)
            te.eventName = QString::fromUtf8(event->labels.event_name);
        else
            te.eventName = QStringLiteral("event %1").arg(event->eventId);

        if (g_objNames && g_objNames->contains(event->threadPointer))
            te.threadName = g_objNames->value(event->threadPointer);
        else if (event->threadPointer == 0xFFFFFFFFu)
            te.threadName = QStringLiteral("ISR");
        else if (event->threadPointer == 0xF0F0F0F0u)
            te.threadName = QStringLiteral("INIT");
        else
            te.threadName = QStringLiteral("0x%1").arg(event->threadPointer, 0, 16);

        g_timeline->append(te);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    scriptPaths_ << QStringLiteral("scripts/isr_enter_count.lua");

    auto *root = new QSplitter(Qt::Vertical, this);

    timelineWidget_ = new TimelineWidget(root);

    statsSplitter_ = new QSplitter(Qt::Horizontal, root);
    ensureStatsPaneCount(scriptPaths_.size());

    root->addWidget(timelineWidget_);
    root->addWidget(statsSplitter_);
    root->setStretchFactor(0, 3);
    root->setStretchFactor(1, 2);

    setCentralWidget(root);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("Open &trace…"), this, &MainWindow::openTrace);
    fileMenu->addAction(tr("Add &statistic script…"), this, &MainWindow::addStatisticScript);
    fileMenu->addAction(tr("Clear statistic scripts"), this, &MainWindow::clearStatisticScripts);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Run analysis"), this, &MainWindow::runAnalysis);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), this, &QWidget::close);

    resize(1100, 750);
    setWindowTitle(tr("TraceX viewer"));
    statusBar()->showMessage(tr("Open a .trx trace to begin"));
}

void MainWindow::setInitialTrace(const QString &path)
{
    tracePath_ = path;
}

void MainWindow::setInitialScript(const QString &path)
{
    if (path.isEmpty())
        return;
    scriptPaths_.clear();
    scriptPaths_ << path;
    ensureStatsPaneCount(scriptPaths_.size());
}

void MainWindow::ensureStatsPaneCount(int n)
{
    while (statsViews_.size() < n) {
        auto *w = new StatsResultWidget(statsSplitter_);
        statsViews_.append(w);
        statsSplitter_->addWidget(w);
    }
    while (statsViews_.size() > n) {
        StatsResultWidget *w = statsViews_.takeLast();
        w->setParent(nullptr);
        delete w;
    }
    for (int i = 0; i < statsViews_.size() && i < scriptPaths_.size(); ++i)
        statsViews_[i]->setScriptPath(scriptPaths_[i]);
}

void MainWindow::openTrace()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open TraceX dump"), QString(),
        tr("TraceX (*.trx);;All files (*)"));
    if (path.isEmpty())
        return;
    tracePath_ = path;
    statusBar()->showMessage(tr("Trace: %1").arg(tracePath_));
    runAnalysis();
}

void MainWindow::addStatisticScript()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Add Lua statistic script"), QString(),
        tr("Lua (*.lua);;All files (*)"));
    if (path.isEmpty())
        return;

    scriptPaths_.append(path);
    ensureStatsPaneCount(scriptPaths_.size());
    statusBar()->showMessage(tr("Added script: %1").arg(path));

    if (!tracePath_.isEmpty())
        runAnalysis();
}

void MainWindow::clearStatisticScripts()
{
    scriptPaths_.clear();
    scriptPaths_ << QStringLiteral("scripts/isr_enter_count.lua");
    ensureStatsPaneCount(scriptPaths_.size());
    statusBar()->showMessage(tr("Statistic scripts reset"));
}

void MainWindow::runAnalysis()
{
    if (tracePath_.isEmpty()) {
        QMessageBox::information(this, tr("TraceX"), tr("Open a trace file first."));
        return;
    }
    if (scriptPaths_.isEmpty()) {
        QMessageBox::information(this, tr("TraceX"), tr("Add at least one statistic script."));
        return;
    }

    if (!parseAndAnalyze())
        statusBar()->showMessage(tr("Analysis failed"));
    else
        statusBar()->showMessage(tr("Analysis done — %1").arg(tracePath_));
}

bool MainWindow::parseAndAnalyze()
{
    QVector<TimelineEvent> timeline;
    QHash<uint32_t, QString> objNames;
    QVector<tracex_stats_handler_t *> handlers;

    g_timeline = &timeline;
    g_objNames = &objNames;
    g_statsHandlers = &handlers;

    ensureStatsPaneCount(scriptPaths_.size());

    /* one stats handler per script */
    for (int i = 0; i < scriptPaths_.size(); ++i) {
        tracex_stats_handler_t *h = nullptr;
        tracex_stats_ret_t sts = tracex_stats_create_new_handler(&h);
        if (sts != TRACEX_STATS_SUCCESS) {
            QMessageBox::critical(this, tr("Stats"),
                                  QString::fromUtf8(tracex_stats_strerror(sts)));
            goto fail;
        }

        sts = tracex_stats_load_script(h, scriptPaths_[i].toUtf8().constData());
        if (sts != TRACEX_STATS_SUCCESS) {
            const char *lua_err = nullptr;
            tracex_stats_get_lua_err_str(h, &lua_err);
            QMessageBox::critical(this, tr("Script"),
                tr("%1\n%2")
                    .arg(scriptPaths_[i])
                    .arg(lua_err ? QString::fromUtf8(lua_err)
                                 : QString::fromUtf8(tracex_stats_strerror(sts))));
            tracex_stats_destroy_handler(&h);
            goto fail;
        }
        handlers.append(h);
        statsViews_[i]->setScriptPath(scriptPaths_[i]);
    }

    if (tracex_create_new_handler(&g_tx) != TRACEX_SUCCESS) {
        QMessageBox::critical(this, tr("TraceX"), tr("Cannot create parse handler"));
        goto fail;
    }

    {
        struct tracex_callbacks cb;
        cb.on_header_parsed = on_header;
        cb.on_object_parsed = on_object;
        cb.on_event_parsed = on_event;
        tracex_register_callbacks(g_tx, &cb);
    }

    labels_engine_object_load_labels(NULL);
    labels_engine_event_load_labels(NULL);
    tracex_register_resolver_function(g_tx, labels_engine_resolve_labels);

    {
        QFile f(tracePath_);
        if (!f.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("File"), tr("Cannot open trace"));
            goto fail;
        }
        QByteArray data = f.readAll();
        char *buf = data.data();
        size_t left = size_t(data.size());
        while (left > 0) {
            size_t n = 0;
            tracex_ret_t st = tracex_parse(g_tx, buf, left, &n);
            if (n == 0)
                break;
            (void)st;
            buf += n;
            left -= n;
        }
    }

    timelineWidget_->setEvents(timeline);

    for (int i = 0; i < handlers.size() && i < statsViews_.size(); ++i) {
        tracex_stats_results *res = nullptr;
        if (tracex_stats_get_results(handlers[i], &res) == TRACEX_STATS_SUCCESS && res) {
            statsViews_[i]->setResults(res);
            tracex_stats_results_free(&res);
        } else {
            statsViews_[i]->setResults(nullptr);
        }
    }

    labels_engine_event_destroy_labels();
    labels_engine_object_destroy_labels();
    tracex_destroy_handler(&g_tx);
    g_tx = nullptr;

    for (int i = 0; i < handlers.size(); ++i)
        tracex_stats_destroy_handler(&handlers[i]);
    handlers.clear();

    g_statsHandlers = nullptr;
    g_timeline = nullptr;
    g_objNames = nullptr;
    return true;

fail:
    labels_engine_event_destroy_labels();
    labels_engine_object_destroy_labels();
    if (g_tx) {
        tracex_destroy_handler(&g_tx);
        g_tx = nullptr;
    }
    for (int i = 0; i < handlers.size(); ++i)
        tracex_stats_destroy_handler(&handlers[i]);
    handlers.clear();
    g_statsHandlers = nullptr;
    g_timeline = nullptr;
    g_objNames = nullptr;
    return false;
}