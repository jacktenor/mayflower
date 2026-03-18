#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QThread>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QScrollBar>
#include <QProgressBar>
#include <QMessageBox>
#include <QTime>
#include <QApplication>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QEventLoop>
#include <QPainter>
#include <QFileDialog>
#include <QListView>
#include <QTreeView>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFileInfo>
#include <QImageReader>
#include <QPixmap>
#include <functional>
#include <QImage>
#include <QProxyStyle>
#include <QTimer>
#include <QDesktopServices>
#include <QScrollArea>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QUrl>
#include <QMovie>
#include <QSettings>
#include <QSpinBox>
#include <QSignalBlocker>
#include <QGroupBox>
#include <QMenu>
#include <QAction>
#include <QStandardItemModel>
#include <cmath>

namespace {

// Unified filter that blocks single-click activation AND Enter/Return on views
class ForceDoubleClickOnlyFilter : public QObject {
public:
    explicit ForceDoubleClickOnlyFilter(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override {
        auto* view = qobject_cast<QAbstractItemView*>(obj);
        if (!view) return QObject::eventFilter(obj, ev);

        switch (ev->type()) {
        case QEvent::MouseButtonDblClick:
            // Allow double-click to pass through normally
            return QObject::eventFilter(obj, ev);

        case QEvent::MouseButtonRelease: {
            // Block single-click from activating/opening
            auto* me = static_cast<QMouseEvent*>(ev);
            if (me->button() == Qt::LeftButton) {
                // Still allow selection, just prevent activation
                return false; // let selection happen, but activation is blocked by NoEditTriggers
            }
            break;
        }

        case QEvent::KeyPress: {
            // Block Enter/Return from activating
            auto* ke = static_cast<QKeyEvent*>(ev);
            if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
                return true; // consume the event
            }
            break;
        }

        default:
            break;
        }

        return QObject::eventFilter(obj, ev);
    }
};

// Watch for new child views being added (handles lazy-loaded views)
class DialogViewWatcher : public QObject {
public:
    explicit DialogViewWatcher(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override {
        if (ev->type() == QEvent::ChildAdded) {
            auto* ce = static_cast<QChildEvent*>(ev);
            if (auto* view = qobject_cast<QAbstractItemView*>(ce->child())) {
                setupView(view);
            }
        }
        return QObject::eventFilter(obj, ev);
    }

private:
    void setupView(QAbstractItemView* view) {
        if (!view) return;

        // Install our filter
        view->installEventFilter(new ForceDoubleClickOnlyFilter(view));

        // Configure view to prevent activation on single-click
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
    }
};

// Proxy style to force double-click activation in item views (e.g., file lists)
class DoubleClickProxyStyle : public QProxyStyle {
public:
    explicit DoubleClickProxyStyle(QStyle *baseStyle) : QProxyStyle(baseStyle) {}

    int styleHint(StyleHint hint, const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const override {
        if (hint == SH_ItemView_ActivateItemOnSingleClick) {
            return 0;  // Force double-click (0 = false)
        }
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

// Re-scale a QImage to fit a box while keeping aspect ratio.
static QPixmap scaledToFit(const QImage& img, const QSize& box) {
    if (img.isNull() || box.isEmpty()) return QPixmap();
    return QPixmap::fromImage(img.scaled(box, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

// Tiny filter to re-render preview when the preview area is resized.
class ResizeUpdateFilter : public QObject {
public:
    explicit ResizeUpdateFilter(std::function<void()> onResize, QObject* parent = nullptr)
        : QObject(parent), m_onResize(std::move(onResize)) {}
protected:
    bool eventFilter(QObject* /*obj*/, QEvent* ev) override {
        if (ev->type() == QEvent::Resize) { if (m_onResize) m_onResize(); }
        return false;
    }
private:
    std::function<void()> m_onResize;
};
}

// Helper function to set up all views in a dialog
// (Keep your ForceDoubleClickOnlyFilter, DialogViewWatcher, DoubleClickProxyStyle,
//  and ResizeUpdateFilter classes exactly as you have them.)

// Helper function to set up all views in a dialog
static void setupFileDialogForDoubleClick(QFileDialog* dlg) {
    if (!dlg) return;

    // Install watcher for future views (handles lazy-loaded/replaced views)
    dlg->installEventFilter(new DialogViewWatcher(dlg));

    // Setup existing views
    auto setupView = [](QAbstractItemView* view) {
        if (!view) return;
        // Your filter that blocks single-click activation and Enter/Return:
        view->installEventFilter(new ForceDoubleClickOnlyFilter(view));
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        // (No setActivationMode here — some Qt versions don’t have it.)
    };

    // Apply to all current views
    for (auto* view : dlg->findChildren<QAbstractItemView*>()) {
        setupView(view);
    }

    // Proxy style disables the platform single-click activation hint
    dlg->setStyle(new DoubleClickProxyStyle(dlg->style()));

    // Re-setup views after show/resize to catch lazily created/replaced views
    QObject::connect(dlg, &QDialog::finished, dlg, &QObject::deleteLater);
    dlg->installEventFilter(new ResizeUpdateFilter([dlg, setupView = std::move(setupView)]() {
        for (auto* view : dlg->findChildren<QAbstractItemView*>()) {
            setupView(view);
        }
    }, dlg));
}

// ===================== Local worker (runs on background thread) =====================
class ThemeTaskWorker : public QObject {
    Q_OBJECT
public:
    ThemeTaskWorker(MainWindow *owner,
                    QString themeName,
                    QString bgPath,
                    QString outDir,
                    bool scaleToScreen,
                    QString gifPath,
                    bool doInstall)
        : m_owner(owner),
        m_theme(std::move(themeName)),
        m_bg(std::move(bgPath)),
        m_out(std::move(outDir)),
        m_gif(std::move(gifPath)),
        m_scale(scaleToScreen),
        m_doInstall(doInstall) {}

signals:
    void progress(QString line);
    void finished(bool ok, QString message);
    void error(QString message);

public slots:
    void run() {
        emit progress("Starting…");

        QString err;
        emit progress("Generating theme files…");
        const QString dir = m_owner->generateThemeFilesBg(
            m_theme, m_bg, m_out, m_scale, m_gif, &err);
        if (dir.isEmpty()) {
            const QString msg = err.isEmpty() ? QStringLiteral("Generation failed.") : err;
            emit error(msg);
            emit finished(false, msg);
            return;
        }

        // --- Preview creation (call via m_owner) ---
{
    QString previewErr;
    if (!m_owner->createPreviewPngFromFrames(dir, m_bg, &previewErr)) {
        emit progress("WARN: Preview generation failed: " + previewErr);
    } else {
        emit progress("Preview image created.");

        // Absolute path to the "good" preview in the theme dir
        const QString outPng = QDir(dir).filePath("preview.png");

        // (1) Remove any accidental stray preview written in parent or CWD
        auto removeStrays = [this, dir, outPng]() {
            const QString parentDir = QFileInfo(dir).absoluteDir().absolutePath();
            const QString strayParent = QDir(parentDir).filePath("preview.png");
            const QString strayCwd    = QDir::current().filePath("preview.png");
            auto tryRemove = [this, &outPng](const QString &path) {
                if (!QFileInfo(path).exists()) return;
                if (QFileInfo(path).absoluteFilePath() == QFileInfo(outPng).absoluteFilePath()) return;
            };
            tryRemove(strayParent);
            tryRemove(strayCwd);
        };
        removeStrays();
        QTimer::singleShot(400, m_owner, removeStrays); // defensive 2nd pass

        // (2) Pop the preview dialog on the GUI thread, pointed at the correct file
        QMetaObject::invokeMethod(m_owner, [this, outPng]() {
            m_owner->showPreviewPopup(outPng);
        }, Qt::QueuedConnection);
    }
}


        if (m_doInstall) {
            emit progress("Installing theme with elevated privileges…");
            if (!m_owner->installThemePrivilegedBg(m_theme, dir, &err)) {
                const QString msg = err.isEmpty() ? QStringLiteral("Install failed.") : err;
                emit error(msg);
                emit finished(false, msg);
                return;
            }
        }

        emit progress("All done.");
        emit finished(true, dir);
    }

private:
    MainWindow *m_owner;
    QString m_theme, m_bg, m_out, m_gif;
    bool m_scale = false;
    bool m_doInstall = false;
};

// Smoothly scales a base pixmap to the label's current size, keeping aspect ratio
class SmoothScalingLabel : public QLabel {
    Q_OBJECT  // <-- add this
public:
    using QLabel::QLabel;

    void setBasePixmap(const QPixmap &pm) {
        base_ = pm;
        applyScale();
    }

protected:
    void resizeEvent(QResizeEvent *e) override {
        QLabel::resizeEvent(e);
        applyScale();
    }

private:
    QPixmap base_;

    void applyScale() {
        if (base_.isNull()) return;

        const qreal dpr = devicePixelRatioF();
        const QSize targetPx = size() * dpr;  // Qt6: QSize * qreal -> QSize
        if (targetPx.isEmpty()) return;

        QPixmap scaled = base_.scaled(
            targetPx,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
        scaled.setDevicePixelRatio(dpr);
        setPixmap(scaled);
    }
};

// ============================== MainWindow impl ==============================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ensureGlobalIndicatorStyle();
    loadDefaultOutDirIntoUi();

    if (ui->labelPreview) {
        QLabel *oldLabel = ui->labelPreview;

        PreviewDragLabel *dragLabel = new PreviewDragLabel(oldLabel->parentWidget());
        dragLabel->setObjectName(oldLabel->objectName());
        dragLabel->setMinimumSize(oldLabel->minimumSize());
        dragLabel->setMaximumSize(oldLabel->maximumSize());
        dragLabel->setSizePolicy(oldLabel->sizePolicy());
        dragLabel->setAlignment(oldLabel->alignment());
        dragLabel->setFrameShape(oldLabel->frameShape());
        dragLabel->setFrameShadow(oldLabel->frameShadow());
        dragLabel->setStyleSheet(oldLabel->styleSheet());
        dragLabel->setScaledContents(oldLabel->hasScaledContents());
        dragLabel->setText(oldLabel->text());

        if (QLayout *parentLayout = oldLabel->parentWidget() ? oldLabel->parentWidget()->layout() : nullptr) {
            parentLayout->replaceWidget(oldLabel, dragLabel);
        }

        oldLabel->hide();
        oldLabel->deleteLater();

        ui->labelPreview = dragLabel;
    }

    if (ui->comboExistingSplashes) {

        connect(ui->comboExistingSplashes, &QComboBox::currentTextChanged,
                this, &MainWindow::onInstalledThemeChanged);

        // Enable right-click on items inside the dropdown list
        if (ui->comboExistingSplashes->view()) {
            ui->comboExistingSplashes->view()->setContextMenuPolicy(Qt::CustomContextMenu);

            connect(ui->comboExistingSplashes->view(),
                    &QWidget::customContextMenuRequested,
                    this,
                    &MainWindow::showInstalledThemeContextMenu);
        }
    }

    setupPreviewMousePositioning();
    setupLivePreviewConnections();

    QTimer::singleShot(0, this, [this]() {
        refreshInstalledThemes();
        updateLivePreviewSources();
    });
}

QString MainWindow::detectCurrentSystemTheme() const
{
    QFileInfo fi("/etc/alternatives/default.plymouth");
    if (!fi.exists())
        return QString();

    const QString target = fi.symLinkTarget();
    if (target.isEmpty())
        return QString();

    QFileInfo targetInfo(target);
    return targetInfo.completeBaseName();
}

QString MainWindow::installedThemesRoot() const
{
    return QStringLiteral("/usr/share/plymouth/themes");
}

QString MainWindow::installedThemePreviewPath(const QString &themeName) const
{
    if (themeName.trimmed().isEmpty())
        return QString();

    return QDir(installedThemesRoot()).filePath(themeName + "/preview.png");
}

void MainWindow::refreshInstalledThemes()
{
    if (!ui || !ui->comboExistingSplashes)
        return;

    QComboBox *combo = ui->comboExistingSplashes;
    const QString previous = combo->currentData().toString().trimmed();

    m_loadingInstalledThemes = true;
    const QSignalBlocker blocker(combo);

    combo->clear();
    combo->addItem(tr("(Select installed splash)"), QString());

    QDir root(installedThemesRoot());
    const QFileInfoList dirs = root.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name | QDir::IgnoreCase
        );

    const QString activeTheme = detectCurrentSystemTheme();

    for (const QFileInfo &fi : dirs) {
        const QString themeName = fi.fileName();
        const QString plyFile = QDir(fi.absoluteFilePath()).filePath(themeName + ".plymouth");
        if (QFileInfo::exists(plyFile)) {
            combo->addItem(displayNameForInstalledTheme(themeName), themeName);
        }
    }

    int idx = -1;
    if (!previous.isEmpty())
        idx = combo->findData(previous);

    if (idx < 0) {
        const QString typedTheme =
            (ui->lineThemeName ? ui->lineThemeName->text().trimmed() : QString());
        if (!typedTheme.isEmpty())
            idx = combo->findData(slugify(typedTheme));
    }

    if (idx < 0) {
        const QString currentTheme = detectCurrentSystemTheme();
        if (!currentTheme.isEmpty())
            idx = combo->findData(currentTheme);
    }

    combo->setCurrentIndex(idx >= 0 ? idx : 0);

    m_loadingInstalledThemes = false;

    onInstalledThemeChanged(combo->currentData().toString());
}

void MainWindow::onInstalledThemeChanged(const QString &themeName)
{
    if (m_loadingInstalledThemes || !ui)
        return;

    const QString clean = themeName.trimmed();

    if (ui->lineThemeName && !clean.isEmpty()) {
        ui->lineThemeName->setText(clean);
    }

    if (ui->lineImagePath && ui->lineImagePath->text().trimmed().isEmpty() &&
        ui->lineAnimPath  && ui->lineAnimPath->text().trimmed().isEmpty()) {
        renderLivePreview();
    }
}

bool MainWindow::computePreviewStageRect(QRect *stageRectOut, QSize *virtualScreenOut) const
{
    if (!ui || !ui->labelPreview)
        return false;

    const QSize labelSize = ui->labelPreview->size();
    if (labelSize.width() < 10 || labelSize.height() < 10)
        return false;

    const qreal stageAspect = 16.0 / 9.0;
    int stageW = labelSize.width();
    int stageH = int(stageW / stageAspect);

    if (stageH > labelSize.height()) {
        stageH = labelSize.height();
        stageW = int(stageH * stageAspect);
    }

    const QRect stageRect(
        (labelSize.width()  - stageW) / 2,
        (labelSize.height() - stageH) / 2,
        stageW,
        stageH
        );

    if (stageRectOut)
        *stageRectOut = stageRect;

    if (virtualScreenOut)
        *virtualScreenOut = QSize(1920, 1080);

    return true;
}

bool MainWindow::currentRenderedAnimationRectInStage(QRect *animRectOut, QRect *stageRectOut) const
{
    if (!ui || !animRectOut)
        return false;

    QRect stageRect;
    QSize virtualScreen;
    if (!computePreviewStageRect(&stageRect, &virtualScreen))
        return false;

    QImage frame;
    if (m_previewMovie && m_previewMovie->isValid())
        frame = m_previewMovie->currentImage();

    if (frame.isNull())
        return false;

    const QSize targetAnimSize = effectiveAnimationPreviewSize(frame.size());
    if (targetAnimSize.isValid() && targetAnimSize != frame.size()) {
        Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio;
        if (auto cb = findChild<QCheckBox*>("chkKeepAspect")) {
            mode = cb->isChecked() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio;
        }
        frame = frame.scaled(targetAnimSize, mode, Qt::SmoothTransformation);
    }

    const int iw = frame.width();
    const int ih = frame.height();
    const int sw = virtualScreen.width();
    const int sh = virtualScreen.height();
    const int margin = 40;

    bool usePercent = false;
    if (auto cb = findChild<QCheckBox*>("usePercentPlacement"))
        usePercent = cb->isChecked();

    int x = 0;
    int y = 0;

    if (usePercent) {
        int xPct = 50;
        int yPct = 50;

        if (auto sp = findChild<QSpinBox*>("xPercentSpin"))
            xPct = qBound(0, sp->value(), 100);

        if (auto sp = findChild<QSpinBox*>("yPercentSpin"))
            yPct = qBound(0, sp->value(), 100);

        x = int((sw - iw) * (xPct / 100.0));
        y = int((sh - ih) * (yPct / 100.0));

        if (x < margin) x = margin;
        if (y < margin) y = margin;
        if (x > sw - iw - margin) x = sw - iw - margin;
        if (y > sh - ih - margin) y = sh - ih - margin;
    } else {
        QString anchor = "center";
        if (auto cb = findChild<QComboBox*>("comboAnimPlacement")) {
            const QString t = cb->currentText().trimmed();
            if (!t.isEmpty())
                anchor = t;
        }

        if      (anchor == "center")         { x = (sw - iw)/2;      y = (sh - ih)/2;      }
        else if (anchor == "bottom-center")  { x = (sw - iw)/2;      y = sh - ih - margin; }
        else if (anchor == "top-center")     { x = (sw - iw)/2;      y = margin;           }
        else if (anchor == "left-center")    { x = margin;           y = (sh - ih)/2;      }
        else if (anchor == "right-center")   { x = sw - iw - margin; y = (sh - ih)/2;      }
        else if (anchor == "bottom-left")    { x = margin;           y = sh - ih - margin; }
        else if (anchor == "bottom-right")   { x = sw - iw - margin; y = sh - ih - margin; }
        else if (anchor == "top-left")       { x = margin;           y = margin;           }
        else if (anchor == "top-right")      { x = sw - iw - margin; y = margin;           }
        else                                 { x = (sw - iw)/2;      y = (sh - ih)/2;      }
    }

    const double sx = double(stageRect.width())  / double(sw);
    const double sy = double(stageRect.height()) / double(sh);

    const int px = stageRect.left() + int(std::round(x * sx));
    const int py = stageRect.top()  + int(std::round(y * sy));
    const int pw = int(std::round(iw * sx));
    const int ph = int(std::round(ih * sy));

    *animRectOut = QRect(px, py, pw, ph);

    if (stageRectOut)
        *stageRectOut = stageRect;

    return true;
}

void MainWindow::setAnimationPercentFromPreviewPoint(const QPoint &previewPos, bool clampToBounds)
{
    if (!ui)
        return;

    QRect stageRect;
    QSize virtualScreen;
    if (!computePreviewStageRect(&stageRect, &virtualScreen))
        return;

    QImage frame;
    if (m_previewMovie && m_previewMovie->isValid())
        frame = m_previewMovie->currentImage();

    if (frame.isNull())
        return;

    const QSize targetAnimSize = effectiveAnimationPreviewSize(frame.size());
    if (targetAnimSize.isValid() && targetAnimSize != frame.size()) {
        Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio;
        if (auto cb = findChild<QCheckBox*>("chkKeepAspect")) {
            mode = cb->isChecked() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio;
        }
        frame = frame.scaled(targetAnimSize, mode, Qt::SmoothTransformation);
    }

    const int iw = frame.width();
    const int ih = frame.height();
    const int sw = virtualScreen.width();
    const int sh = virtualScreen.height();
    const int margin = 40;

    const double sx = double(sw) / double(stageRect.width());
    const double sy = double(sh) / double(stageRect.height());

    int x = int(std::round((previewPos.x() - stageRect.left()) * sx));
    int y = int(std::round((previewPos.y() - stageRect.top())  * sy));

    if (clampToBounds) {
        if (x < margin) x = margin;
        if (y < margin) y = margin;
        if (x > sw - iw - margin) x = sw - iw - margin;
        if (y > sh - ih - margin) y = sh - ih - margin;
    }

    int xPct = 0;
    int yPct = 0;

    if (sw - iw > 0)
        xPct = int(std::round((double(x) / double(sw - iw)) * 100.0));
    if (sh - ih > 0)
        yPct = int(std::round((double(y) / double(sh - ih)) * 100.0));

    xPct = qBound(0, xPct, 100);
    yPct = qBound(0, yPct, 100);

    if (auto cb = findChild<QCheckBox*>("usePercentPlacement")) {
        if (!cb->isChecked())
            cb->setChecked(true);
    }

    if (auto sp = findChild<QSpinBox*>("xPercentSpin")) {
        const QSignalBlocker b(sp);
        sp->setValue(xPct);
    }

    if (auto sp = findChild<QSpinBox*>("yPercentSpin")) {
        const QSignalBlocker b(sp);
        sp->setValue(yPct);
    }

    renderLivePreview();
}

QString MainWindow::displayNameForInstalledTheme(const QString &themeName) const
{
    if (themeName.trimmed().isEmpty())
        return themeName;

    const QString activeTheme = detectCurrentSystemTheme();
    if (themeName == activeTheme)
        return themeName + tr(" (active)");

    return themeName;
}

void MainWindow::setupPreviewMousePositioning()
{
    if (!ui || !ui->labelPreview)
        return;

    auto dragLabel = qobject_cast<PreviewDragLabel*>(ui->labelPreview);
    if (!dragLabel)
        return;

    dragLabel->setMouseTracking(true);

    connect(dragLabel, &PreviewDragLabel::mousePressed, this,
            [this, dragLabel](const QPoint &pos)
            {
                QRect animRect;
                if (currentRenderedAnimationRectInStage(&animRect) && animRect.contains(pos)) {
                    m_previewDragging = true;
                    m_dragHotspot = pos - animRect.topLeft();
                    dragLabel->setCursor(Qt::ClosedHandCursor);
                }
            });

    connect(dragLabel, &PreviewDragLabel::mouseMoved, this,
            [this, dragLabel](const QPoint &pos, Qt::MouseButtons buttons)
            {
                QRect animRect;
                const bool overAnim =
                    currentRenderedAnimationRectInStage(&animRect) && animRect.contains(pos);

                if (m_previewDragging && (buttons & Qt::LeftButton)) {
                    setAnimationPercentFromPreviewPoint(pos - m_dragHotspot, true);
                    dragLabel->setCursor(Qt::ClosedHandCursor);
                    return;
                }

                if (overAnim) {
                    dragLabel->setCursor(Qt::OpenHandCursor);
                } else {
                    dragLabel->unsetCursor();
                }
            });

    connect(dragLabel, &PreviewDragLabel::mouseReleased, this,
            [this, dragLabel](const QPoint &pos)
            {
                Q_UNUSED(pos);

                m_previewDragging = false;

                QRect animRect;
                const bool overAnim =
                    currentRenderedAnimationRectInStage(&animRect) && animRect.contains(pos);

                if (overAnim) {
                    dragLabel->setCursor(Qt::OpenHandCursor);
                } else {
                    dragLabel->unsetCursor();
                }
            });
}

bool MainWindow::activateInstalledThemePrivileged(const QString &themeName, QString *errOut)
{
    auto fail = [&](const QString &e) {
        if (errOut) *errOut = e;
        appendLog("ERROR: " + e);
        return false;
    };

    const QString theme = themeName.trimmed();
    if (theme.isEmpty())
        return fail(tr("No installed splash was selected."));

    auto esc = [](QString s) {
        s.replace("\\", "\\\\");
        s.replace("\"", "\\\"");
        return s;
    };

    const QString safeTheme = esc(theme);

    QString sh;
    sh += "set -e\n";
    sh += "set -o pipefail\n";
    sh += "THEME=\"" + safeTheme + "\"\n";
    sh += "DST=\"/usr/share/plymouth/themes/$THEME\"\n";
    sh += "PLY=\"$DST/$THEME.plymouth\"\n";
    sh += "LOGTAG='[activate]'\n";

    sh += "echo \"$LOGTAG Activating installed theme: $THEME\"\n";
    sh += "if [ ! -f \"$PLY\" ]; then echo \"$LOGTAG ERROR: $PLY not found\"; exit 1; fi\n";

    sh += "if command -v update-alternatives >/dev/null 2>&1; then\n";
    sh += "  update-alternatives --install /usr/share/plymouth/themes/default.plymouth default.plymouth \"$PLY\" 100 || true\n";
    sh += "  update-alternatives --set default.plymouth \"$PLY\" || true\n";
    sh += "fi\n";

    sh += "if command -v plymouth-set-default-theme >/dev/null 2>&1; then\n";
    sh += "  plymouth-set-default-theme \"$THEME\" || true\n";
    sh += "fi\n";

    sh += "if command -v update-initramfs >/dev/null 2>&1; then\n";
    sh += "  update-initramfs -u\n";
    sh += "fi\n";

    sh += "echo \"$LOGTAG Activation finished successfully.\"\n";

    return runRootScriptStreaming(sh, errOut);
}

bool MainWindow::deleteInstalledThemePrivileged(const QString &themeName, QString *errOut)
{
    auto fail = [&](const QString &e) {
        if (errOut) *errOut = e;
        appendLog("ERROR: " + e);
        return false;
    };

    const QString theme = themeName.trimmed();
    if (theme.isEmpty())
        return fail(tr("No installed splash was selected."));

    auto esc = [](QString s) {
        s.replace("\\", "\\\\");
        s.replace("\"", "\\\"");
        return s;
    };

    const QString safeTheme = esc(theme);

    QString sh;
    sh += "set -e\n";
    sh += "set -o pipefail\n";
    sh += "THEME=\"" + safeTheme + "\"\n";
    sh += "DST=\"/usr/share/plymouth/themes/$THEME\"\n";
    sh += "PLY=\"$DST/$THEME.plymouth\"\n";
    sh += "SPINNER=\"/usr/share/plymouth/themes/spinner/spinner.plymouth\"\n";
    sh += "CURRENT=\"$(readlink -f /etc/alternatives/default.plymouth 2>/dev/null || true)\"\n";
    sh += "LOGTAG='[delete]'\n";

    sh += "echo \"$LOGTAG Deleting installed theme: $THEME\"\n";
    sh += "if [ ! -d \"$DST\" ]; then echo \"$LOGTAG ERROR: $DST not found\"; exit 1; fi\n";

    sh += "if [ \"$CURRENT\" = \"$PLY\" ] && [ -f \"$SPINNER\" ]; then\n";
    sh += "  echo \"$LOGTAG Selected theme is active. Switching temporarily to spinner first.\"\n";
    sh += "  if command -v update-alternatives >/dev/null 2>&1; then\n";
    sh += "    update-alternatives --install /usr/share/plymouth/themes/default.plymouth default.plymouth \"$SPINNER\" 100 || true\n";
    sh += "    update-alternatives --set default.plymouth \"$SPINNER\" || true\n";
    sh += "  fi\n";
    sh += "  if command -v plymouth-set-default-theme >/dev/null 2>&1; then\n";
    sh += "    plymouth-set-default-theme spinner || true\n";
    sh += "  fi\n";
    sh += "fi\n";

    sh += "if command -v update-alternatives >/dev/null 2>&1; then\n";
    sh += "  update-alternatives --remove default.plymouth \"$PLY\" || true\n";
    sh += "fi\n";

    sh += "rm -rf \"$DST\"\n";

    sh += "if command -v update-initramfs >/dev/null 2>&1; then\n";
    sh += "  update-initramfs -u\n";
    sh += "fi\n";

    sh += "echo \"$LOGTAG Deletion finished successfully.\"\n";

    return runRootScriptStreaming(sh, errOut);
}

void MainWindow::showInstalledThemeContextMenu(const QPoint &pos)
{
    if (!ui || !ui->comboExistingSplashes)
        return;

    auto view = ui->comboExistingSplashes->view();
    if (!view)
        return;

    QModelIndex index = view->indexAt(pos);
    ui->comboExistingSplashes->setCurrentIndex(index.row());

    if (!index.isValid())
        return;

    const QString themeName = index.data().toString().trimmed();
    if (themeName.isEmpty())
        return;

    QMenu menu(this);

    QAction *deleteAction  = menu.addAction(tr("Delete Splash"));
    QAction *refreshAction = menu.addAction(tr("Refresh List"));

    QAction *chosen = menu.exec(view->viewport()->mapToGlobal(pos));
    if (!chosen)
        return;

    if (chosen == refreshAction) {
        refreshInstalledThemes();
        return;
    }

    if (chosen == deleteAction) {

        const auto answer = QMessageBox::question(
            this,
            tr("Delete Splash"),
            tr("Delete installed splash \"%1\"?\n\nThis removes it from /usr/share/plymouth/themes.")
                .arg(themeName)
            );

        if (answer != QMessageBox::Yes)
            return;

        setUiBusy(true);

        QString err;
        const bool ok = deleteInstalledThemePrivileged(themeName, &err);

        setUiBusy(false);

        if (!ok) {
            QMessageBox::critical(this, tr("Delete Failed"), err);
            return;
        }

        appendLog("Deleted splash: " + themeName);

        refreshInstalledThemes();
        updateLivePreviewSources();
    }
}

    // (Optional) If you also added a dedicated QLabel named logLabel in the UI,
    // you can keep or remove that block. The overlay above makes it unnecessary.


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupLivePreviewConnections()
{
    auto queueRender = [this]() {
        QTimer::singleShot(0, this, [this]() {
            renderLivePreview();
        });
    };

    auto queueReload = [this]() {
        QTimer::singleShot(0, this, [this]() {
            updateLivePreviewSources();
        });
    };

    if (ui->lineImagePath) {
        connect(ui->lineImagePath, &QLineEdit::textChanged, this,
                [queueReload](const QString&) { queueReload(); });
    }

    if (ui->lineAnimPath) {
        connect(ui->lineAnimPath, &QLineEdit::textChanged, this,
                [queueReload](const QString&) { queueReload(); });
    }

    if (ui->lineOutDir) {
        connect(ui->lineOutDir, &QLineEdit::textChanged, this,
                [queueRender](const QString&) { queueRender(); });
    }

    if (auto cb = findChild<QComboBox*>("comboAnimPlacement")) {
        connect(cb, &QComboBox::currentTextChanged, this,
                [queueRender](const QString&) { queueRender(); });
    }

    if (auto cb = findChild<QCheckBox*>("usePercentPlacement")) {
        connect(cb, &QCheckBox::toggled, this,
                [queueRender](bool) { queueRender(); });
    }

    if (auto sp = findChild<QSpinBox*>("xPercentSpin")) {
        connect(sp, qOverload<int>(&QSpinBox::valueChanged), this,
                [queueRender](int) { queueRender(); });
    }

    if (auto sp = findChild<QSpinBox*>("yPercentSpin")) {
        connect(sp, qOverload<int>(&QSpinBox::valueChanged), this,
                [queueRender](int) { queueRender(); });
    }

    if (auto sp = findChild<QSpinBox*>("spinAnimW")) {
        connect(sp, qOverload<int>(&QSpinBox::valueChanged), this,
                [queueRender](int) { queueRender(); });
    }

    if (auto sp = findChild<QSpinBox*>("spinAnimH")) {
        connect(sp, qOverload<int>(&QSpinBox::valueChanged), this,
                [queueRender](int) { queueRender(); });
    }

    if (auto cb = findChild<QCheckBox*>("chkKeepAspect")) {
        connect(cb, &QCheckBox::toggled, this,
                [queueRender](bool) { queueRender(); });
    }

    if (ui->labelPreview) {
        ui->labelPreview->setAlignment(Qt::AlignCenter);
        ui->labelPreview->setScaledContents(false);
        ui->labelPreview->setText(tr("Live preview will appear here"));
        ui->labelPreview->setStyleSheet(
            "QLabel {"
            "  border: 1px solid #555;"
            "  background: #111;"
            "  color: #bbb;"
            "}"
            );

        ui->labelPreview->installEventFilter(
            new ResizeUpdateFilter([this]() {
                renderLivePreview();
            }, ui->labelPreview)
            );
    }
}

QSize MainWindow::effectiveAnimationPreviewSize(const QSize &srcSize) const
{
    int targetW = 0;
    int targetH = 0;
    bool keepAspect = true;

    if (auto sp = findChild<QSpinBox*>("spinAnimW"))
        targetW = sp->value();

    if (auto sp = findChild<QSpinBox*>("spinAnimH"))
        targetH = sp->value();

    if (auto cb = findChild<QCheckBox*>("chkKeepAspect"))
        keepAspect = cb->isChecked();

    if (srcSize.isEmpty())
        return QSize();

    int w = targetW;
    int h = targetH;

    if (keepAspect) {
        if (w > 0 && h > 0)
            return srcSize.scaled(w, h, Qt::KeepAspectRatio);

        if (w > 0)
            return srcSize.scaled(w, 100000, Qt::KeepAspectRatio);

        if (h > 0)
            return srcSize.scaled(100000, h, Qt::KeepAspectRatio);

        return srcSize;
    }

    if (w <= 0 && h <= 0)
        return srcSize;

    if (w <= 0)
        w = srcSize.width();

    if (h <= 0)
        h = srcSize.height();

    return QSize(w, h);
}

bool MainWindow::rebuildLivePreviewFrames(QString *errOut)
{
    const QString animPath = ui->lineAnimPath ? ui->lineAnimPath->text().trimmed() : QString();
    if (animPath.isEmpty() || !QFileInfo::exists(animPath)) {
        if (errOut) *errOut = tr("Animation file not selected.");
        return false;
    }

    const QString outDir = ui->lineOutDir ? ui->lineOutDir->text().trimmed() : QString();
    if (outDir.isEmpty() || !QDir(outDir).exists()) {
        if (errOut) *errOut = tr("Output directory not selected.");
        return false;
    }

    const QString previewBaseDir = QDir(outDir).filePath(".mayflower_live_preview");
    const QString framesDir = QDir(previewBaseDir).filePath("frames");

    QDir().mkpath(framesDir);

    // Clean old preview frames
    QDir fd(framesDir);
    const QFileInfoList oldFiles = fd.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &fi : oldFiles) {
        QFile::remove(fi.absoluteFilePath());
    }

    int frameCount = 0;
    QString gifErr;
    if (!extractGifFramesPlymouth(animPath, framesDir, &gifErr, &frameCount)) {
        if (errOut) *errOut = gifErr.isEmpty() ? tr("Failed to extract animation frames.") : gifErr;
        return false;
    }

    int targetW = 0;
    int targetH = 0;
    bool keepAspect = true;

    if (auto sp = findChild<QSpinBox*>("spinAnimW")) targetW = sp->value();
    if (auto sp = findChild<QSpinBox*>("spinAnimH")) targetH = sp->value();
    if (auto cb = findChild<QCheckBox*>("chkKeepAspect")) keepAspect = cb->isChecked();

    if (targetW > 0 || targetH > 0) {
        QString resizeErr;
        if (!resizeAnimationFrames(framesDir, targetW, targetH, keepAspect, &resizeErr)) {
            if (errOut) *errOut = resizeErr.isEmpty() ? tr("Failed to resize preview frames.") : resizeErr;
            return false;
        }
    }

    m_livePreviewFramesDir = framesDir;
    return true;
}

void MainWindow::updateLivePreview()
{
    updateLivePreviewSources();
}

void MainWindow::updateLivePreviewSources()
{
    if (!ui || !ui->labelPreview)
        return;

    const QString bgPath =
        ui->lineImagePath ? ui->lineImagePath->text().trimmed() : QString();
    const QString gifPath =
        ui->lineAnimPath ? ui->lineAnimPath->text().trimmed() : QString();

    m_previewBackground = QImage();
    if (!bgPath.isEmpty() && QFileInfo::exists(bgPath)) {
        QImageReader r(bgPath);
        r.setAutoTransform(true);
        m_previewBackground = r.read();
    }

    if (m_previewMovie) {
        m_previewMovie->stop();
        m_previewMovie->deleteLater();
        m_previewMovie = nullptr;
    }
    m_previewMoviePath.clear();

    if (!gifPath.isEmpty() && QFileInfo::exists(gifPath)) {
        auto *movie = new QMovie(gifPath, QByteArray(), this);
        movie->setCacheMode(QMovie::CacheAll);

        if (movie->isValid()) {
            m_previewMovie = movie;
            m_previewMoviePath = gifPath;

            connect(movie, &QMovie::frameChanged, this, [this](int) {
                renderLivePreview();
            });

            movie->start();
        } else {
            movie->deleteLater();
        }
    }

    renderLivePreview();
}

void MainWindow::renderLivePreview()
{
    if (!ui || !ui->labelPreview)
        return;

    const QSize labelSize = ui->labelPreview->size();
    if (labelSize.width() < 10 || labelSize.height() < 10)
        return;

    const bool hasBackground = !m_previewBackground.isNull();
    const bool hasMovieFrame =
        (m_previewMovie && m_previewMovie->isValid() && !m_previewMovie->currentImage().isNull());

    // --- If no custom files are loaded, try showing preview.png from installed theme ---
    if (!hasBackground && !hasMovieFrame) {
        QString installedTheme;
        if (ui->comboExistingSplashes)
            installedTheme = ui->comboExistingSplashes->currentData().toString().trimmed();

        if (!installedTheme.isEmpty()) {
            const QString previewPath = installedThemePreviewPath(installedTheme);
            QImage installedPreview(previewPath);
            if (!installedPreview.isNull()) {
                ui->labelPreview->setText(QString());
                ui->labelPreview->setPixmap(
                    QPixmap::fromImage(installedPreview).scaled(
                        labelSize,
                        Qt::KeepAspectRatio,
                        Qt::SmoothTransformation
                        )
                    );
                return;
            }
        }

        QPixmap pm(":/icons/appicon.png");
        if (!pm.isNull()) {
            ui->labelPreview->setText(QString());
            ui->labelPreview->setPixmap(
                pm.scaled(labelSize,
                          Qt::KeepAspectRatio,
                          Qt::SmoothTransformation)
                );
        } else {
            ui->labelPreview->setPixmap(QPixmap());
            ui->labelPreview->setText(tr("Live preview will appear here"));
        }
        return;
    }

    QImage output(labelSize, QImage::Format_ARGB32_Premultiplied);
    output.fill(QColor("#111111"));

    QPainter outPainter(&output);
    outPainter.setRenderHint(QPainter::Antialiasing, true);
    outPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const qreal stageAspect = 16.0 / 9.0;
    int stageW = labelSize.width();
    int stageH = int(stageW / stageAspect);

    if (stageH > labelSize.height()) {
        stageH = labelSize.height();
        stageW = int(stageH * stageAspect);
    }

    const QRect stageRect(
        (labelSize.width()  - stageW) / 2,
        (labelSize.height() - stageH) / 2,
        stageW,
        stageH
        );

    outPainter.fillRect(stageRect, Qt::black);

    const QSize virtualScreen(1920, 1080);

    QImage stage(virtualScreen, QImage::Format_ARGB32_Premultiplied);
    stage.fill(Qt::black);

    {
        QPainter p(&stage);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);

        if (!m_previewBackground.isNull()) {
            QImage bgScaled = m_previewBackground.scaled(
                virtualScreen,
                Qt::KeepAspectRatioByExpanding,
                Qt::SmoothTransformation
                );

            const int bgX = (virtualScreen.width()  - bgScaled.width())  / 2;
            const int bgY = (virtualScreen.height() - bgScaled.height()) / 2;
            p.drawImage(bgX, bgY, bgScaled);
        }

        QImage frame;
        if (m_previewMovie && m_previewMovie->isValid()) {
            frame = m_previewMovie->currentImage();
        }

        if (!frame.isNull()) {
            const QSize targetAnimSize = effectiveAnimationPreviewSize(frame.size());
            if (targetAnimSize.isValid() && targetAnimSize != frame.size()) {
                Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio;
                if (auto cb = findChild<QCheckBox*>("chkKeepAspect")) {
                    mode = cb->isChecked() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio;
                }

                frame = frame.scaled(
                    targetAnimSize,
                    mode,
                    Qt::SmoothTransformation
                    );
            }

            const int iw = frame.width();
            const int ih = frame.height();

            bool usePercent = false;
            if (auto cb = findChild<QCheckBox*>("usePercentPlacement"))
                usePercent = cb->isChecked();

            int x = 0;
            int y = 0;
            const int sw = virtualScreen.width();
            const int sh = virtualScreen.height();
            const int margin = 40;

            if (usePercent) {
                int xPct = 50;
                int yPct = 50;

                if (auto sp = findChild<QSpinBox*>("xPercentSpin"))
                    xPct = qBound(0, sp->value(), 100);

                if (auto sp = findChild<QSpinBox*>("yPercentSpin"))
                    yPct = qBound(0, sp->value(), 100);

                x = int((sw - iw) * (xPct / 100.0));
                y = int((sh - ih) * (yPct / 100.0));

                if (x < margin) x = margin;
                if (y < margin) y = margin;
                if (x > sw - iw - margin) x = sw - iw - margin;
                if (y > sh - ih - margin) y = sh - ih - margin;
            } else {
                QString anchor = "center";
                if (auto cb = findChild<QComboBox*>("comboAnimPlacement")) {
                    const QString t = cb->currentText().trimmed();
                    if (!t.isEmpty())
                        anchor = t;
                }

                if      (anchor == "center")         { x = (sw - iw)/2;      y = (sh - ih)/2;      }
                else if (anchor == "bottom-center")  { x = (sw - iw)/2;      y = sh - ih - margin; }
                else if (anchor == "top-center")     { x = (sw - iw)/2;      y = margin;           }
                else if (anchor == "left-center")    { x = margin;           y = (sh - ih)/2;      }
                else if (anchor == "right-center")   { x = sw - iw - margin; y = (sh - ih)/2;      }
                else if (anchor == "bottom-left")    { x = margin;           y = sh - ih - margin; }
                else if (anchor == "bottom-right")   { x = sw - iw - margin; y = sh - ih - margin; }
                else if (anchor == "top-left")       { x = margin;           y = margin;            }
                else if (anchor == "top-right")      { x = sw - iw - margin; y = margin;            }
                else                                 { x = (sw - iw)/2;      y = (sh - ih)/2;      }
            }

            p.drawImage(QPoint(x, y), frame);
        }
    }

    outPainter.drawImage(stageRect, stage);
    outPainter.end();

    ui->labelPreview->setText(QString());
    ui->labelPreview->setPixmap(QPixmap::fromImage(output));
}



// --- Add this method to mainwindow.cpp ---
void MainWindow::ensureGlobalIndicatorStyle()
{
    // Clear any per-widget styles that might override the app stylesheet
    // (Only touches QCheckBox/QRadioButton descendants of this window)
    const auto checkboxes = this->findChildren<QCheckBox *>();
    for (QCheckBox *cb : checkboxes) {
        if (!cb->styleSheet().isEmpty())
            cb->setStyleSheet(QString());
    }
}


QFileDialog* MainWindow::createFileDialogWithImagePreview(const QString& title,
                                                          const QString& startDir,
                                                          const QStringList& nameFilters)
{
    auto *dlg = new QFileDialog(this, title, startDir);
    dlg->setOption(QFileDialog::DontUseNativeDialog, true);
    dlg->setFileMode(QFileDialog::ExistingFile);
    dlg->setNameFilters(nameFilters);
    dlg->setViewMode(QFileDialog::Detail);

    // Setup double-click enforcement
    setupFileDialogForDoubleClick(dlg);

    // Build right-side preview panel
    auto *previewPane = new QWidget(dlg);
    auto *v = new QVBoxLayout(previewPane);
    v->setContentsMargins(8,8,8,8);
    v->setSpacing(8);

    auto *imgLabel = new QLabel(previewPane);
    imgLabel->setAlignment(Qt::AlignCenter);
    imgLabel->setMinimumSize(260, 180);
    imgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imgLabel->setStyleSheet("QLabel { border: 1px solid #555; background: #111; }");
    imgLabel->setScaledContents(false);  // We'll handle scaling ourselves

    auto *metaLabel = new QLabel(previewPane);
    metaLabel->setWordWrap(true);
    metaLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    metaLabel->setStyleSheet("QLabel { color: #bbb; }");

    v->addWidget(imgLabel, 1);
    v->addWidget(metaLabel, 0);
    previewPane->setLayout(v);

    // Insert panel into QFileDialog's grid layout
    if (auto *gl = qobject_cast<QGridLayout*>(dlg->layout())) {
        int rows = gl->rowCount();
        int col  = gl->columnCount();
        gl->addWidget(previewPane, 0, col, rows, 1);
        gl->setColumnStretch(col, 1);
    } else {
        dlg->layout()->addWidget(previewPane);
    }

    // Shared state for preview
    auto currentImage = std::make_shared<QImage>();
    auto currentPath  = std::make_shared<QString>();
    auto currentMovie = std::make_shared<QPointer<QMovie>>();

    // Helper to stop any running animation
    auto stopAnimation = [currentMovie]() {
        if (*currentMovie) {
            (*currentMovie)->stop();
            (*currentMovie)->deleteLater();
            *currentMovie = nullptr;
        }
    };

    // Render function for static images
    auto renderStatic = [=]() {
        if (currentImage->isNull()) {
            imgLabel->setPixmap(QPixmap());
            metaLabel->setText("No preview");
            return;
        }
        imgLabel->setPixmap(scaledToFit(*currentImage, imgLabel->size()));
        QFileInfo fi(*currentPath);
        const QString info = QString("%1\n%2 × %3  •  %4 KB")
                                .arg(fi.fileName())
                                .arg(currentImage->width()).arg(currentImage->height())
                                .arg(qMax<qint64>(1, fi.size() / 1024));
        metaLabel->setText(info);
    };

    // Helper to check if file is animated
    auto isAnimatedFormat = [](const QString& path) -> bool {
        const QString lower = path.toLower();
        return lower.endsWith(".gif") ||
               lower.endsWith(".webp") ||
               lower.endsWith(".apng");
    };

    // Update preview when selection changes
    QObject::connect(dlg, &QFileDialog::currentChanged, dlg, [=](const QString& path){
        *currentPath = path;
        stopAnimation();  // Stop any previous animation

        QFileInfo fi(path);
        if (!fi.exists() || !fi.isFile()) {
            *currentImage = QImage();
            renderStatic();
            return;
        }

        // Try to play as animation first if it's an animated format
        if (isAnimatedFormat(path)) {
            auto* movie = new QMovie(path, QByteArray(), dlg);

            if (movie->isValid() && movie->frameCount() > 1) {
                // It's a valid animation with multiple frames
                *currentMovie = movie;

                // Scale the movie to fit the label
                QSize originalSize = movie->scaledSize();
                if (originalSize.isEmpty()) {
                    originalSize = movie->currentPixmap().size();
                }

                // Calculate scaled size maintaining aspect ratio
                QSize labelSize = imgLabel->size();
                QSize scaledSize = originalSize.scaled(labelSize, Qt::KeepAspectRatio);
                movie->setScaledSize(scaledSize);

                imgLabel->setMovie(movie);
                movie->start();

                // Update metadata
                const QString info = QString("%1\n%2 × %3  •  %4 KB\n%5 frames")
                                        .arg(fi.fileName())
                                        .arg(originalSize.width())
                                        .arg(originalSize.height())
                                        .arg(qMax<qint64>(1, fi.size() / 1024))
                                        .arg(movie->frameCount());
                metaLabel->setText(info);
                return;
            } else {
                // Not a valid multi-frame animation, clean up
                movie->deleteLater();
            }
        }

        // Fall back to static image preview
        QImageReader r(path);
        r.setAutoTransform(true);
        if (!r.canRead()) {
            *currentImage = QImage();
            renderStatic();
            return;
        }

        QImage img = r.read();
        *currentImage = img;
        renderStatic();
    });

    // Re-render on resize to keep crisp scaling
    auto resizeHandler = [=]() {
        if (*currentMovie) {
            // Rescale the animated movie
            QSize originalSize = (*currentMovie)->scaledSize();
            if (originalSize.isEmpty()) {
                originalSize = (*currentMovie)->currentPixmap().size();
            }
            QSize labelSize = imgLabel->size();
            QSize scaledSize = originalSize.scaled(labelSize, Qt::KeepAspectRatio);
            (*currentMovie)->setScaledSize(scaledSize);
        } else {
            // Rescale static image
            renderStatic();
        }
    };

    previewPane->installEventFilter(new ResizeUpdateFilter(resizeHandler, previewPane));
    imgLabel->installEventFilter(new ResizeUpdateFilter(resizeHandler, imgLabel));

    // Clean up when dialog is destroyed
    QObject::connect(dlg, &QObject::destroyed, [stopAnimation]() {
        stopAnimation();
    });

    return dlg;
}


// NEW: Modal popup that shows the freshly created preview.png.
// - Scales to fit window but preserves aspect ratio.
// - Includes basic zoom via Ctrl+MouseWheel / trackpad (Qt default on ScrollArea).
void MainWindow::showPreviewPopup(const QString &pngPath)
{
    if (pngPath.isEmpty() || !QFileInfo::exists(pngPath)) {
        appendLog("Preview popup aborted: file missing: " + pngPath);
        return;
    }

    auto *dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("Theme Preview — %1").arg(QFileInfo(pngPath).fileName()));
    dlg->resize(860, 600);

    auto *lay = new QVBoxLayout(dlg);
    auto *scroll = new QScrollArea(dlg);
    scroll->setWidgetResizable(true);
    auto *imgLabel = new QLabel(scroll);
    imgLabel->setAlignment(Qt::AlignCenter);
    scroll->setWidget(imgLabel);
    lay->addWidget(scroll);

    auto loadFresh = [imgLabel, scroll](const QString &path) {
        QImageReader r(path);
        r.setAutoTransform(true);
        const QImage img = r.read();
        if (img.isNull()) {
            imgLabel->setText(QObject::tr("Failed to load preview image."));
            return;
        }
        const QSize avail = scroll->viewport()->size();
        imgLabel->setPixmap(QPixmap::fromImage(img).scaled(avail, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    };

    // Run after show
    QTimer::singleShot(0, dlg, [loadFresh, pngPath]{ loadFresh(pngPath); });
    // Refit on scroll range changes (resize)
    QObject::connect(scroll->verticalScrollBar(), &QScrollBar::rangeChanged, dlg, [loadFresh, pngPath]{ loadFresh(pngPath); });
    QObject::connect(scroll->horizontalScrollBar(), &QScrollBar::rangeChanged, dlg, [loadFresh, pngPath]{ loadFresh(pngPath); });

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Close, dlg);
    auto *openFolderBtn = bb->addButton(tr("Open Folder"), QDialogButtonBox::ActionRole);
    QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::close);
    QObject::connect(openFolderBtn, &QPushButton::clicked, dlg, [pngPath]{
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(pngPath).absolutePath()));
    });
    lay->addWidget(bb);

    dlg->setModal(true);
    dlg->show();
}

void MainWindow::ensureSingleClickSelectOnly(QFileDialog *dlg)
{
    if (!dlg) return;

    auto applyToAllViews = [dlg]() {
        int n = 0;
        const auto views = dlg->findChildren<QAbstractItemView*>();
        for (auto *v : views) {
            if (!v) continue;
            // Install your existing filter (idempotent) and push the proxy style
            if (!v->isWindow()) {
                v->installEventFilter(new ForceDoubleClickOnlyFilter(v));
                v->setEditTriggers(QAbstractItemView::NoEditTriggers);
                v->setSelectionBehavior(QAbstractItemView::SelectRows);
                v->setSelectionMode(QAbstractItemView::SingleSelection);
                // The key line: apply the proxy style to the *view itself*.
                v->setStyle(new DoubleClickProxyStyle(v->style()));
                ++n;
            }
        }
        qInfo() << "[Mayflower] ensureSingleClickSelectOnly applied to" << n << "views.";
    };

    // 0) Run once after the dialog has been constructed and laid out
    QTimer::singleShot(0, dlg, applyToAllViews);

    // 1) Re-apply when the user navigates (views/models can be rebuilt lazily)
    QObject::connect(dlg, &QFileDialog::directoryEntered, dlg, applyToAllViews);
    QObject::connect(dlg, &QFileDialog::filterSelected,   dlg, applyToAllViews);

    // 2) Re-apply on resize/show (some desktops replace views after show)
    dlg->installEventFilter(new ResizeUpdateFilter(applyToAllViews, dlg));
}

// NEW: Convenience wrapper that pulls paths from your UI and refreshes preview.
bool MainWindow::refreshPreviewFromCurrentUI(const QString &framesDir, QString *errOut)
{
    const QString bgPath  = (ui->lineImagePath  ? ui->lineImagePath->text().trimmed()  : QString());
    const QString outDir  = (ui->lineOutDir  ? ui->lineOutDir->text().trimmed()  : QString());

    if (bgPath.isEmpty() || !QFileInfo::exists(bgPath)) {
        if (errOut) *errOut = "Background image path not set or missing.";
        return false;
    }
    if (outDir.isEmpty() || !QDir(outDir).exists()) {
        if (errOut) *errOut = "Output directory path not set or missing.";
        return false;
    }

    return generatePreviewAfterResize(bgPath, framesDir, outDir, errOut);
}

bool MainWindow::resizeAnimationFrames(const QString &framesDir,
                                       int targetWidth,
                                       int targetHeight,
                                       bool keepAspect,
                                       QString *errOut)
{
    const QString im = findImageMagick();
    if (im.isEmpty()) {
        if (errOut) *errOut = "ImageMagick not found (need 'magick' or 'convert' in PATH).";
        return false;
    }

    // Gather frames
    QStringList frameFiles;
    for (QDirIterator it(framesDir, QStringList() << "frame_*.png", QDir::Files);
         it.hasNext();) {
        frameFiles << it.next();
    }
    if (frameFiles.isEmpty()) {
        if (errOut) *errOut = "No frames found to resize in: " + framesDir;
        return false;
    }
    frameFiles.sort();

    // Load first frame to know original aspect (needed for some cases)
    QImage first(frameFiles.first());
    if (first.isNull()) {
        if (errOut) *errOut = "Failed to load first frame to determine aspect.";
        return false;
    }
    const int srcW = qMax(1, first.width());
    const int srcH = qMax(1, first.height());

    // Build geometry string smartly
    int w = targetWidth;
    int h = targetHeight;

    QString geometry;
    if (keepAspect) {
        // Accept one-dimension entry (0 means "auto")
        if (w > 0 && h > 0) {
            geometry = QString("%1x%2").arg(w).arg(h);          // keep aspect, fit within box
        } else if (w > 0 && h <= 0) {
            geometry = QString("%1x").arg(w);                   // width only
        } else if (h > 0 && w <= 0) {
            geometry = QString("x%1").arg(h);                   // height only
        } else {
            // nothing requested
            if (errOut) *errOut = "No target size provided.";
            return true; // treat as "nothing to do" rather than hard-fail
        }
    } else {
        // No aspect: require both sides; if only one is given, compute the other from source aspect
        if (w <= 0 && h <= 0) {
            if (errOut) *errOut = "No target size provided.";
            return true; // nothing to do
        }
        if (w <= 0 && h > 0) {
            w = qMax(1, int(double(h) * double(srcW) / double(srcH)));
        } else if (h <= 0 && w > 0) {
            h = qMax(1, int(double(w) * double(srcH) / double(srcW)));
        }
        geometry = QString("%1x%2!").arg(w).arg(h);             // force exact
    }

    // Temp dir
    const QString tempDir = QDir(framesDir).filePath(".tmp_resized");
    if (QDir(tempDir).exists()) {
        for (const QFileInfo &fi : QDir(tempDir).entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
            QFile::remove(fi.absoluteFilePath());
        QDir().rmdir(tempDir);
    }
    if (!QDir().mkpath(tempDir)) {
        if (errOut) *errOut = "Failed to create temp dir: " + tempDir;
        return false;
    }

    bool allSuccess = true;
    QString so, se;

    // Process each frame → PNG32 (keep alpha)
    for (const QString &framePath : frameFiles) {
        QFileInfo fi(framePath);
        const QString tempOutput = QDir(tempDir).filePath(fi.fileName());

        QStringList args;
        args << framePath
             << "-resize" << geometry
             << "-background" << "none"
             << "-alpha" << "on"
             << "-strip"
             << QString("PNG32:%1").arg(tempOutput);

        if (!runProgram(im, args, &so, &se)) {
            appendLog(QString("Failed to resize frame: %1 - %2").arg(fi.fileName(), se));
            allSuccess = false;
            continue;
        }
    }

    if (!allSuccess) {
        if (errOut) *errOut = "Some frames failed to resize. Check log for details.";
        for (const QFileInfo &fi : QDir(tempDir).entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
            QFile::remove(fi.absoluteFilePath());
        QDir().rmdir(tempDir);
        return false;
    }

    // Swap in resized frames
    for (const QString &framePath : frameFiles) {
        QFileInfo fi(framePath);
        const QString tempOutput = QDir(tempDir).filePath(fi.fileName());

        if (!QFile::remove(framePath)) {
            appendLog(QString("Warning: Could not remove original frame: %1").arg(fi.fileName()));
        }
        if (!QFile::rename(tempOutput, framePath)) {
            appendLog(QString("ERROR: Failed to move resized frame into place: %1").arg(fi.fileName()));
            if (errOut) *errOut = "Failed to finalize resized frames.";
            // Best effort cleanup:
            for (const QFileInfo &f2 : QDir(tempDir).entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
                QFile::remove(f2.absoluteFilePath());
            QDir().rmdir(tempDir);
            return false;
        }
    }

    // Cleanup
    for (const QFileInfo &fi : QDir(tempDir).entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
        QFile::remove(fi.absoluteFilePath());
    QDir().rmdir(tempDir);

    return true;
}

// NEW: Composite a representative (resized) frame over the background to build preview.png
// - bgPath: path to selected background image
// - framesDir: directory containing the (now resized) frame_*.png files
// - outDir: output directory for theme; preview will be written as outDir/preview.png
// Returns true on success and logs progress/errors.
bool MainWindow::generatePreviewAfterResize(const QString &bgPath,
                                            const QString &framesDir,
                                            const QString &outDir,
                                            QString *errOut)
{
    if (bgPath.isEmpty() || !QFileInfo::exists(bgPath)) {
        if (errOut) *errOut = "Background image missing.";
        return false;
    }
    QDir d(framesDir);
    if (!d.exists()) {
        if (errOut) *errOut = "Frames directory missing: " + framesDir;
        return false;
    }

    QStringList frames = d.entryList(QStringList() << "frame_*.png", QDir::Files, QDir::Name);
    if (frames.isEmpty()) {
        if (errOut) *errOut = "No frames found in: " + framesDir;
        return false;
    }
    frames.sort();
    const QString firstFrame = d.filePath(frames.first());

    int xPct = 50, yPct = 50;
    if (auto sx = findChild<QSpinBox*>("xPercentSpin")) xPct = qBound(0, sx->value(), 100);
    if (auto sy = findChild<QSpinBox*>("yPercentSpin")) yPct = qBound(0, sy->value(), 100);

    QImage bg(bgPath), fr(firstFrame);
    if (bg.isNull() || fr.isNull()) {
        if (errOut) *errOut = "Failed to read images for preview.";
        return false;
    }

    const int bgW = bg.width(),  bgH = bg.height();
    const int frW = fr.width(),  frH = fr.height();
    const int centerX = (bgW * xPct) / 100;
    const int centerY = (bgH * yPct) / 100;
    const int posX = qMax(0, centerX - frW/2);
    const int posY = qMax(0, centerY - frH/2);

    const QString magick = findImageMagick();
    if (magick.isEmpty()) {
        if (errOut) *errOut = "ImageMagick not found (need 'magick' or 'convert' in PATH).";
        return false;
    }

    const QString outPng = QDir(outDir).filePath("preview.png");
    QStringList args;
    args << bgPath
         << firstFrame
         << "-geometry" << QString("+%1+%2").arg(posX).arg(posY)
         << "-compose"  << "over"
         << "-alpha"    << "on"
         << "-composite"
         << "-strip"
         << QString("PNG32:%1").arg(outPng);

    QString so, se;
    if (!runProgram(magick, args, &so, &se)) {
        if (errOut) *errOut = se.isEmpty() ? "ImageMagick failed creating preview." : se;
        appendLog("Preview generation error: " + *errOut);
        return false;
    }

    appendLog("Preview updated after resize: " + outPng);


    // --- Stray preview cleanup (parent dir AND current working dir) ---
    auto removeStrays = [this, outDir, outPng]() {
        const QString parentDir = QFileInfo(outDir).absoluteDir().absolutePath();
        const QString strayParent = QDir(parentDir).filePath("preview.png");
        const QString strayCwd    = QDir::current().filePath("preview.png");

        auto tryRemove = [this, &outPng](const QString &path) {
            if (!QFileInfo(path).exists()) return;
            if (QFileInfo(path).absoluteFilePath() == QFileInfo(outPng).absoluteFilePath()) return;
            if (QFile::remove(path)) {
                appendLog("Removed stray preview: " + path);
            } else {
                appendLog("WARNING: Could not remove stray preview: " + path);
            }
        };
        tryRemove(strayParent);
        tryRemove(strayCwd);
    };

    // Remove immediately…
    removeStrays();
    // …and remove again shortly after (defensive: if legacy writer runs after us)
    QTimer::singleShot(250, this, removeStrays);

    return true;
}

bool MainWindow::createPreviewPngFromFrames(const QString& themeDir,
                                            const QString& backgroundPath,
                                            QString* outErrorMessage)
{
    // Load background (acts as our "screen" for preview)
    QImage bg(backgroundPath);
    if (bg.isNull()) {
        if (outErrorMessage) *outErrorMessage = tr("Failed to load background: %1").arg(backgroundPath);
        qWarning() << "[preview] Failed to load background:" << backgroundPath;
        return false;
    }
    const int sw = bg.width();
    const int sh = bg.height();

    // Pick a representative animation frame from themeDir/frames
    const QString framesPath = QDir(themeDir).filePath("frames");
    QDir fd(framesPath);
    if (!fd.exists()) {
        if (outErrorMessage) *outErrorMessage = tr("Frames folder not found: %1").arg(framesPath);
        qWarning() << "[preview] Missing frames dir:" << framesPath;
        return false;
    }
    fd.setNameFilters(QStringList() << "frame_*.png" << "*.png" << "*.webp" << "*.jpg" << "*.jpeg" << "*.bmp");
    fd.setFilter(QDir::Files | QDir::Readable | QDir::NoSymLinks);
    fd.setSorting(QDir::Name | QDir::LocaleAware);
    const QFileInfoList list = fd.entryInfoList();
    if (list.isEmpty()) {
        if (outErrorMessage) *outErrorMessage = tr("No frames found in: %1").arg(framesPath);
        qWarning() << "[preview] No frames in" << framesPath;
        return false;
    }
    const QString framePath = list.first().absoluteFilePath();

    QImage frame(framePath);
    if (frame.isNull()) {
        if (outErrorMessage) *outErrorMessage = tr("Failed to load animation frame: %1").arg(framePath);
        qWarning() << "[preview] Failed to load frame:" << framePath;
        return false;
    }
    const int iw = frame.width();
    const int ih = frame.height();

    // ----- Mirror your script's placement logic -----
    // UI controls (same names as in your script generation)
    bool usePercent = false;
    if (auto cbx = findChild<QCheckBox*>("usePercentPlacement"))
        usePercent = cbx->isChecked();

    int xPct = 50, yPct = 50;  // defaults match script
    if (auto spx = findChild<QSpinBox*>("xPercentSpin"))
        xPct = qBound(0, spx->value(), 100);
    if (auto spy = findChild<QSpinBox*>("yPercentSpin"))
        yPct = qBound(0, spy->value(), 100);

    const int margin = 40;     // same margin you use in the script
    int x = 0, y = 0;

    if (usePercent) {
        // Percentage mode (identical math)
        x = int((sw - iw) * (xPct / 100.0));
        y = int((sh - ih) * (yPct / 100.0));
        if (x < margin) x = margin;
        if (y < margin) y = margin;
        if (x > sw - iw - margin) x = sw - iw - margin;
        if (y > sh - ih - margin) y = sh - ih - margin;
    } else {
        // Anchor presets (identical to your script)
        QString anchor = "center";
        if (auto cb = findChild<QComboBox*>("comboAnimPlacement")) {
            const auto t = cb->currentText().trimmed();
            if (!t.isEmpty()) anchor = t;
        }
        if      (anchor == "center")         { x = (sw - iw)/2;        y = (sh - ih)/2;        }
        else if (anchor == "bottom-center")  { x = (sw - iw)/2;        y = sh - ih - margin;   }
        else if (anchor == "top-center")     { x = (sw - iw)/2;        y = margin;             }
        else if (anchor == "left-center")    { x = margin;             y = (sh - ih)/2;        }
        else if (anchor == "right-center")   { x = sw - iw - margin;   y = (sh - ih)/2;        }
        else if (anchor == "bottom-left")    { x = margin;             y = sh - ih - margin;   }
        else if (anchor == "bottom-right")   { x = sw - iw - margin;   y = sh - ih - margin;   }
        else if (anchor == "top-left")       { x = margin;             y = margin;             }
        else if (anchor == "top-right")      { x = sw - iw - margin;   y = margin;             }
        else                                  { x = (sw - iw)/2;        y = (sh - ih)/2;        }
    }

    // Compose: background + frame at computed (x,y)
    QImage canvas(bg.size(), QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);
    {
        QPainter p(&canvas);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        p.drawImage(QPoint(0, 0), bg);
        p.drawImage(QPoint(x, y), frame);
    }

    const QString outPath = QDir(themeDir).filePath("preview.png");
    if (!canvas.save(outPath, "PNG")) {
        if (outErrorMessage) *outErrorMessage = tr("Failed to save preview: %1").arg(outPath);
        qWarning() << "[preview] Failed to save:" << outPath;
        return false;
    }

    qInfo() << "[preview] Wrote" << outPath << " (screen:" << sw << "x" << sh
            << ", frame:" << iw << "x" << ih << ", pos:" << x << "," << y << ")";
    return true;
}

// ---------------- Thread-safe logger ----------------
void MainWindow::appendLog(const QString& msg)
{
    if (!ui) return;

    QPlainTextEdit* log = ui->textLog;
    if (!log) return;

    const QString line = QTime::currentTime().toString("HH:mm:ss  ") + msg;
    QPointer<QPlainTextEdit> safeLog = log;

    QMetaObject::invokeMethod(
        this,
        [safeLog, line]() {
            if (!safeLog) return;
            safeLog->appendPlainText(line);

            if (QScrollBar* sb = safeLog->verticalScrollBar()) {
                const int maxv = sb->maximum();
                if (sb->value() != maxv) sb->setValue(maxv);
            }
        },
        Qt::QueuedConnection
    );
}


void MainWindow::setUiBusy(bool busy)
{
    if (auto w = findChild<QWidget*>("centralwidget")) w->setEnabled(!busy);
    if (auto b = findChild<QPushButton*>("btnGenerate")) b->setEnabled(!busy);
    if (auto b = findChild<QPushButton*>("btnInstall"))  b->setEnabled(!busy);
    if (auto bar = findChild<QProgressBar*>("progressBar")) {
        bar->setRange(0, 0);
        bar->setVisible(busy);
    }
}

QString MainWindow::generateThemeFilesBg(const QString &themeName,
                                         const QString &bgImagePath,
                                         const QString &outBaseDir,
                                         bool scaleToScreen,
                                         const QString &animGifPath,
                                         QString *err)
{
    return generatePlymouthThemeFilesCore(themeName, bgImagePath, outBaseDir, scaleToScreen, animGifPath, err);
}

bool MainWindow::installThemePrivilegedBg(const QString &themeName,
                                          const QString &themeDir,
                                          QString *err)
{
    return installThemePrivilegedCoreStreaming(themeName, themeDir, err);
}

// NEW: Normalize extracted frame names to frame_0000.png, frame_0001.png, ...
static bool normalizeFrameFilenames(const QString &framesDir, QString *err)
{
    QStringList names = QDir(framesDir).entryList(QStringList() << "*.png", QDir::Files);
    if (names.isEmpty()) return true;

    QList<QFileInfo> infos;
    infos.reserve(names.size());
    for (const QString &n : std::as_const(names))
        infos << QFileInfo(QDir(framesDir).filePath(n));

    std::sort(infos.begin(), infos.end(),
              [](const QFileInfo &a, const QFileInfo &b){
                  if (a.lastModified() != b.lastModified())
                      return a.lastModified() < b.lastModified();
                  return a.fileName() < b.fileName();
              });

    for (int i = 0; i < infos.size(); ++i) {
        const QString src = infos[i].absoluteFilePath();
        const QString dst = QDir(framesDir).filePath(QString("frame_%1.png")
                           .arg(i, 4, 10, QLatin1Char('0')));
        if (src == dst) continue;
        if (QFile::exists(dst) && !QFile::remove(dst)) {
            if (err) *err = QString("Cannot remove existing %1").arg(dst);
            return false;
        }
        if (!QFile::rename(src, dst)) {
            if (err) *err = QString("Failed to rename %1 -> %2").arg(src, dst);
            return false;
        }
    }
    return true;
}

QString MainWindow::generatePlymouthThemeFilesCore(const QString &themeName,
                                                   const QString &bgImagePath,
                                                   const QString &outBaseDir,
                                                   bool /*scaleToScreen*/,
                                                   const QString &animGifPath,
                                                   QString *errOut)
{
    auto fail = [&](const QString &e)->QString{
        if (errOut) *errOut = e;
        appendLog("ERROR: " + e);
        return {};
    };

    if (themeName.trimmed().isEmpty())  return fail("Theme Name is required.");
    if (outBaseDir.trimmed().isEmpty()) return fail("Output folder is required.");

    const QString base = slugify(themeName);
    const QString themeDir = QDir(outBaseDir).filePath(base);
    if (!QDir().mkpath(themeDir)) return fail("Could not create theme directory: " + themeDir);

    QStringList filesList;

    // ---------- .plymouth ----------
    const QString plyPath = QDir(themeDir).filePath(base + ".plymouth");
    QString ply = QString(
        "[Plymouth Theme]\n"
        "Name=%1\n"
        "Description=Theme generated by Mayflower helper\n"
        "ModuleName=script\n\n"
        "[script]\n"
        "ImageDir=/usr/share/plymouth/themes/%2\n"
        "ScriptFile=/usr/share/plymouth/themes/%2/%2.script\n"
    ).arg(themeName, base);

    // ---------- .script ----------
    QString scr;
    scr += "Window.SetBackgroundTopColor(0.0,0.0,0.0);\n";
    scr += "Window.SetBackgroundBottomColor(0.0,0.0,0.0);\n";

    const bool haveBg  = !bgImagePath.trimmed().isEmpty();
    const bool haveGif = !animGifPath.trimmed().isEmpty();

    // Background: convert to a Plymouth-safe opaque PNG (PNG24)
    if (haveBg) {
        const QString bgDst = QDir(themeDir).filePath("background.png");
        QString convErr;
        if (!convertImageToPlymouthPng(bgImagePath, bgDst, &convErr)) {
            return fail("Background conversion failed: " + convErr);
        }
        filesList << "background.png";

        scr += "bg_img=Image(\"background.png\"); bg=Sprite(bg_img);\n";
        scr += "sw=Window.GetWidth(); sh=Window.GetHeight();\n";
        scr += "bw=bg_img.GetWidth(); bh=bg_img.GetHeight();\n";
        scr += "bg.SetX((sw-bw)/2); bg.SetY((sh-bh)/2); bg.SetZ(-100);\n";
    }

    // Read desired FPS from UI if available; default 25
    int fps = 25;
    if (ui && ui->fpsSpin) {
        fps = qBound(1, ui->fpsSpin->value(), 60);
    }
    int step = qBound(1, qRound(50.0 / qMax(1, fps)), 10);

    bool usePercent = false;
    int xPct = 50, yPct = 50;
    if (auto cbx = findChild<QCheckBox*>("usePercentPlacement"))
        usePercent = cbx->isChecked();
    if (auto spx = findChild<QSpinBox*>("xPercentSpin"))
        xPct = qBound(0, spx->value(), 100);
    if (auto spy = findChild<QSpinBox*>("yPercentSpin"))
        yPct = qBound(0, spy->value(), 100);

    const int margin = 40;


    int frameCount = 0;
    if (haveGif) {
        const QString framesDir = QDir(themeDir).filePath("frames");
        QString gifErr;
        if (!extractGifFramesPlymouth(animGifPath, framesDir, &gifErr, &frameCount)) {
            return fail(gifErr);
        }

        // Read UI for resize request
        bool shouldResize = false;
        int targetW = 0, targetH = 0;
        bool keepAspect = true;

        if (auto sw = findChild<QSpinBox*>("spinAnimW"))  targetW = sw->value();      // 0 = auto
        if (auto sh = findChild<QSpinBox*>("spinAnimH"))  targetH = sh->value();      // 0 = auto
        if (auto ck = findChild<QCheckBox*>("chkKeepAspect")) keepAspect = ck->isChecked();

        // Resize if user specified at least one dimension
        if (targetW > 0 || targetH > 0) {
            shouldResize = true;
        }

        if (shouldResize) {
            QString resizeErr;
            if (!resizeAnimationFrames(framesDir, targetW, targetH, keepAspect, &resizeErr)) {
                appendLog("WARNING: Frame resize failed: " + resizeErr);
                // Continue with originals
            } else {
                appendLog(QString("Resized frames to %1x%2 (%3)")
                          .arg(targetW > 0 ? QString::number(targetW) : "auto")
                          .arg(targetH > 0 ? QString::number(targetH) : "auto")
                          .arg(keepAspect ? "keep aspect" : "no aspect"));
            }
        }

        // Continue with existing code...
        QStringList frameFiles;
        for (QDirIterator it(framesDir, QStringList() << "frame_*.png", QDir::Files);
             it.hasNext();) frameFiles << it.next();
        frameFiles.sort();

        scr += "frames=[]; sprite=Sprite();\n";
        for (int i = 0; i < frameFiles.size(); ++i) {
            const QString rel = "frames/" + QFileInfo(frameFiles[i]).fileName();
            scr += QString("frames[%1]=Image(\"%2\");\n").arg(i).arg(rel);
            filesList << rel;
        }
        scr += QString("count=%1; idx=0;\n").arg(frameFiles.size());
        scr += "sprite.SetImage(frames[0]);\n";

        scr += "sw=Window.GetWidth(); sh=Window.GetHeight();\n";
        scr += "iw=frames[0].GetWidth(); ih=frames[0].GetHeight();\n";

        if (usePercent) {
            scr += QString("xPct=%1; yPct=%2; margin=%3;\n")
                       .arg(xPct).arg(yPct).arg(margin);
            scr += "x = (sw - iw) * (xPct/100.0);\n";
            scr += "y = (sh - ih) * (yPct/100.0);\n";
            scr += "if (x < margin) x = margin;\n";
            scr += "if (y < margin) y = margin;\n";
            scr += "if (x > sw - iw - margin) x = sw - iw - margin;\n";
            scr += "if (y > sh - ih - margin) y = sh - ih - margin;\n";
        } else {
            QString anchor = "center";
            if (auto cb = findChild<QComboBox*>("comboAnimPlacement")) {
                const auto t = cb->currentText().trimmed();
                if (!t.isEmpty()) anchor = t;
            }
            auto place = [&](const QString &a)->QString{
                QString s = "margin=" + QString::number(margin) + "; anchor=\"" + a + "\";\n";
                s += "if(anchor==\"center\"){x=(sw-iw)/2;y=(sh-ih)/2;}";
                s += "else if(anchor==\"bottom-center\"){x=(sw-iw)/2;y=sh-ih-margin;}";
                s += "else if(anchor==\"top-center\"){x=(sw-iw)/2;y=margin;}";
                s += "else if(anchor==\"left-center\"){x=margin;y=(sh-ih)/2;}";
                s += "else if(anchor==\"right-center\"){x=sw-iw-margin;y=(sh-ih)/2;}";
                s += "else if(anchor==\"bottom-left\"){x=margin;y=sh-ih-margin;}";
                s += "else if(anchor==\"bottom-right\"){x=sw-iw-margin;y=sh-ih-margin;}";
                s += "else if(anchor==\"top-left\"){x=margin;y=margin;}";
                s += "else if(anchor==\"top-right\"){x=sw-iw-margin;y=margin;}";
                s += "else{x=(sw-iw)/2;y=(sh-ih)/2;}\n";
                return s;
            };
            scr += place(anchor);
        }

        scr += "sprite.SetX(x); sprite.SetY(y); sprite.SetZ(-50);\n";

        scr += QString("tick=0; step=%1;\n").arg(step);
        scr += "fun _advance_() { idx=(idx+1)%count; sprite.SetImage(frames[idx]); }\n";
        scr += "fun _refresh_() { tick=tick+1; if ((tick % step)==0) _advance_(); }\n";
        scr += "Plymouth.SetRefreshFunction(_refresh_);\n";
    } else {
        scr += "// No animation provided.\n";
    }

    scr += "progress=ProgressBar();\n";
    scr += "progress.SetX(Window.GetWidth()/6);\n";
    scr += "progress.SetY(Window.GetHeight()*5/6);\n";
    scr += "progress.SetWidth(Window.GetWidth()*2/3);\n";

    QString err;
    if (!writeTextFile(plyPath, ply, &err)) return fail("Write .plymouth failed: " + err);
    const QString scrPath = QDir(themeDir).filePath(base + ".script");
    if (!writeTextFile(scrPath, scr, &err)) return fail("Write .script failed: " + err);

    if (!filesList.isEmpty()) {
        QString filesBody;
        for (const QString &rel : filesList) filesBody += rel + "\n";
        writeTextFile(QDir(themeDir).filePath("files.txt"), filesBody, nullptr);
    }

    appendLog("Theme files generated at: " + themeDir);
    return themeDir;
}

bool MainWindow::runRootScriptStreaming(const QString &scriptContent,
                                        QString *errOut)
{
    QTemporaryFile tmp;
    tmp.setAutoRemove(false);
    if (!tmp.open()) {
        if (errOut) *errOut = "Cannot create temporary script file.";
        appendLog("ERROR: Cannot create temporary script file.");
        return false;
    }
    tmp.write(scriptContent.toUtf8());
    tmp.close();

    QProcess proc;
    proc.setProgram("pkexec");
    proc.setArguments({"bash", tmp.fileName()});
    proc.setProcessChannelMode(QProcess::MergedChannels);

    proc.start();
    if (!proc.waitForStarted(10000)) {
        if (errOut) *errOut = "Failed to start pkexec.";
        appendLog("ERROR: Failed to start pkexec.");
        QFile::remove(tmp.fileName());
        return false;
    }

    QByteArray buf;
    while (true) {
        if (!proc.waitForReadyRead(100)) {
            if (proc.state() == QProcess::NotRunning) break;
        } else {
            buf.append(proc.readAll());
            int idx;
            while ((idx = buf.indexOf('\n')) >= 0) {
                const QByteArray line = buf.left(idx);
                buf.remove(0, idx + 1);
                appendLog(QString::fromLocal8Bit(line));
            }
        }
    }
    buf.append(proc.readAll());
    if (!buf.isEmpty())
        appendLog(QString::fromLocal8Bit(buf));

    int code = proc.exitCode();
    QFile::remove(tmp.fileName());

    if (code != 0) {
        const QString msg = QString("Installer script exited with code %1.").arg(code);
        if (errOut) *errOut = msg;
        appendLog("ERROR: " + msg);
        return false;
    }
    return true;
}

bool MainWindow::installThemePrivilegedCoreStreaming(const QString &themeName,
                                                     const QString &themeDir,
                                                     QString *errOut)
{
    auto fail = [&](const QString &e){ if (errOut) *errOut = e; appendLog("ERROR: " + e); return false; };

    if (themeName.trimmed().isEmpty()) return fail("Theme Name required.");
    if (!QFileInfo::exists(themeDir))  return fail("Theme dir does not exist: " + themeDir);

    const QString base = slugify(themeName);
    const QString dst  = "/usr/share/plymouth/themes/" + base;
    const QString ply  = dst + "/" + base + ".plymouth";

    QString sh;
    sh += "set -e\n";
    sh += "set -o pipefail\n";
    sh += "THEME=\"" + base + "\"\n";
    sh += "SRC=\"" + themeDir + "\"\n";
    sh += "DST=\"/usr/share/plymouth/themes/$THEME\"\n";
    sh += "PLY=\"$DST/$THEME.plymouth\"\n";
    sh += "LOGTAG='[install]'\n";

    sh += "echo \"$LOGTAG Installing theme: $THEME\"\n";
    sh += "echo \"$LOGTAG Source: $SRC\"\n";
    sh += "echo \"$LOGTAG Destination: $DST\"\n";

    sh += "echo \"$LOGTAG Creating destination directory…\"\n";
    sh += "mkdir -p \"$DST\"\n";

    sh += "echo \"$LOGTAG Copying files…\"\n";
    sh += "cp -a \"$SRC\"/. \"$DST\"/\n";

    sh += "echo \"$LOGTAG Files copied. Verifying key file: $PLY\"\n";
    sh += "if [ ! -f \"$PLY\" ]; then echo \"$LOGTAG ERROR: $PLY not found\"; exit 1; fi\n";

    sh += "echo \"$LOGTAG Selecting default Plymouth theme…\"\n";
    sh += "if command -v update-alternatives >/dev/null 2>&1; then\n";
    sh += "  update-alternatives --install /usr/share/plymouth/themes/default.plymouth default.plymouth \"$PLY\" 100 || true\n";
    sh += "  update-alternatives --set default.plymouth \"$PLY\" || true\n";
    sh += "fi\n";
    sh += "if command -v plymouth-set-default-theme >/dev/null 2>&1; then\n";
    sh += "  plymouth-set-default-theme \"$THEME\" || true\n";
    sh += "fi\n";

    sh += "echo \"$LOGTAG Rebuilding initramfs…\"\n";
    sh += "if command -v update-initramfs >/dev/null 2>&1; then\n";
    sh += "  echo \"$LOGTAG Using update-initramfs -u\"\n";
    sh += "  update-initramfs -u\n";
    sh += "elif command -v mkinitcpio >/dev/null 2>&1; then\n";
    sh += "  echo \"$LOGTAG Using mkinitcpio -P\"\n";
    sh += "  mkinitcpio -P\n";
    sh += "elif command -v dracut >/dev/null 2>&1; then\n";
    sh += "  echo \"$LOGTAG Using dracut -f\"\n";
    sh += "  dracut -f\n";
    sh += "elif command -v mkinitramfs >/dev/null 2>&1; then\n";
    sh += "  echo \"$LOGTAG Using mkinitramfs (kernel=$(uname -r))\"\n";
    sh += "  mkinitramfs -o /boot/initrd.img-\"$(uname -r)\" \"$(uname -r)\"\n";
    sh += "elif command -v mkinitrd >/dev/null 2>&1; then\n";
    sh += "  echo \"$LOGTAG Using mkinitrd\"\n";
    sh += "  mkinitrd\n";
    sh += "else\n";
    sh += "  echo \"$LOGTAG WARNING: No initramfs tool found; please rebuild initramfs manually.\"\n";
    sh += "fi\n";

    sh += "echo \"$LOGTAG Ensuring GRUB has 'splash' flag…\"\n";
    sh += "if [ -f /etc/default/grub ]; then\n";
    sh += "  if ! grep -q 'splash' /etc/default/grub; then\n";
    sh += "    sed -i 's/GRUB_CMDLINE_LINUX_DEFAULT=\"\\([^\"]*\\)\"/GRUB_CMDLINE_LINUX_DEFAULT=\"\\1 splash\"/' /etc/default/grub || true\n";
    sh += "    echo \"$LOGTAG Added 'splash' to GRUB_CMDLINE_LINUX_DEFAULT\"\n";
    sh += "  else\n";
    sh += "    echo \"$LOGTAG 'splash' already present\"\n";
    sh += "  fi\n";
    sh += "  if command -v update-grub >/dev/null 2>&1; then\n";
    sh += "    echo \"$LOGTAG Running update-grub\"\n";
    sh += "    update-grub || true\n";
    sh += "  elif command -v grub-mkconfig >/dev/null 2>&1; then\n";
    sh += "    echo \"$LOGTAG Running grub-mkconfig -o /boot/grub/grub.cfg\"\n";
    sh += "    grub-mkconfig -o /boot/grub/grub.cfg || true\n";
    sh += "  else\n";
    sh += "    echo \"$LOGTAG WARNING: No GRUB regeneration tool found; please update your bootloader manually.\"\n";
    sh += "  fi\n";
    sh += "else\n";
    sh += "  echo \"$LOGTAG WARNING: /etc/default/grub not found; skipping GRUB changes\"\n";
    sh += "fi\n";

    sh += "echo \"$LOGTAG Install finished successfully.\"\n";

    return runRootScriptStreaming(sh, errOut);
}

void MainWindow::startGenerateInBackground()
{
    const QString theme = ui->lineThemeName ? ui->lineThemeName->text() : QString();
    const QString bg    = ui->lineImagePath ? ui->lineImagePath->text() : QString();
    const QString out   = ui->lineOutDir    ? ui->lineOutDir->text()    : QString();
    const bool    scale = ui->chkScaleToScreen ? ui->chkScaleToScreen->isChecked() : false;
    const QString gif   = ui->lineAnimPath ? ui->lineAnimPath->text() : QString();

    setUiBusy(true);
    appendLog("Queued: generate…");

    auto *thread = new QThread(this);
    auto *worker = new ThemeTaskWorker(this, theme, bg, out, scale, gif, false);
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &ThemeTaskWorker::run);
    connect(worker, &ThemeTaskWorker::progress, this, &MainWindow::onWorkerProgress, Qt::QueuedConnection);
    connect(worker, &ThemeTaskWorker::error,    this, &MainWindow::onWorkerError,    Qt::QueuedConnection);
    connect(worker, &ThemeTaskWorker::finished, this, &MainWindow::onWorkerFinished, Qt::QueuedConnection);
    connect(worker, &ThemeTaskWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
}

void MainWindow::startGenerateAndInstallInBackground()
{
    const QString theme = ui->lineThemeName ? ui->lineThemeName->text() : QString();
    const QString bg    = ui->lineImagePath ? ui->lineImagePath->text() : QString();
    const QString out   = ui->lineOutDir    ? ui->lineOutDir->text()    : QString();
    const bool    scale = ui->chkScaleToScreen ? ui->chkScaleToScreen->isChecked() : false;
    const QString gif   = ui->lineAnimPath ? ui->lineAnimPath->text() : QString();

    setUiBusy(true);
    appendLog("Queued: generate + install…");

    auto *thread = new QThread(this);
    auto *worker = new ThemeTaskWorker(this, theme, bg, out, scale, gif, true);
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &ThemeTaskWorker::run);
    connect(worker, &ThemeTaskWorker::progress, this, &MainWindow::onWorkerProgress, Qt::QueuedConnection);
    connect(worker, &ThemeTaskWorker::error,    this, &MainWindow::onWorkerError,    Qt::QueuedConnection);
    connect(worker, &ThemeTaskWorker::finished, this, &MainWindow::onWorkerFinished, Qt::QueuedConnection);
    connect(worker, &ThemeTaskWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
}

void MainWindow::on_btnGenerate_clicked()
{
    this->setProperty("lastTask", "generate");
    startGenerateInBackground();
}

void MainWindow::on_btnInstall_clicked()
{
    const QString selectedInstalled =
        (ui && ui->comboExistingSplashes)
            ? ui->comboExistingSplashes->currentData().toString().trimmed()
            : QString();

    const QString bg =
        (ui && ui->lineImagePath) ? ui->lineImagePath->text().trimmed() : QString();

    const QString gif =
        (ui && ui->lineAnimPath) ? ui->lineAnimPath->text().trimmed() : QString();

    if (!selectedInstalled.isEmpty() && bg.isEmpty() && gif.isEmpty()) {
        setUiBusy(true);
        appendLog("Activating installed splash: " + selectedInstalled);

        QString err;
        const bool ok = activateInstalledThemePrivileged(selectedInstalled, &err);

        setUiBusy(false);

        if (!ok) {
            QMessageBox::critical(
                this,
                tr("Activation Failed"),
                err.isEmpty() ? tr("Failed to activate installed splash.") : err
                );
            return;
        }

        appendLog("Activated installed splash: " + selectedInstalled);
        refreshInstalledThemes();
        renderLivePreview();

        QMessageBox::information(
            this,
            tr("Success"),
            tr("Activated installed splash \"%1\".").arg(selectedInstalled)
            );
        return;
    }

    this->setProperty("lastTask", "install");
    startGenerateAndInstallInBackground();
}

void MainWindow::onWorkerProgress(const QString &line)
{
    appendLog(line);
}

void MainWindow::onWorkerError(const QString &line)
{
    appendLog("ERROR: " + line);
}

void MainWindow::onWorkerFinished(bool ok, const QString &info)
{
    appendLog(ok ? "Task finished." : "Task failed.");
    if (ok && !info.isEmpty())
        appendLog("Result: " + info);

    setUiBusy(false);

    const QString kind = this->property("lastTask").toString();

    if (ok) {
        refreshInstalledThemes();

        const QString wanted =
            ui && ui->lineThemeName ? slugify(ui->lineThemeName->text().trimmed()) : QString();

        if (!wanted.isEmpty() && ui && ui->comboExistingSplashes) {
            const int idx = ui->comboExistingSplashes->findData(wanted);
            if (idx >= 0)
                ui->comboExistingSplashes->setCurrentIndex(idx);
        }

        renderLivePreview();

        QString title = tr("Success");
        QString text;

        if (kind == QLatin1String("generate")) {
            text = tr("Files generated successfully!");
        } else if (kind == QLatin1String("install")) {
            text = tr("Installation complete!");
        } else {
            title = tr("Done");
            text = tr("Task completed.");
        }

        const QString details = info.trimmed();
        QMetaObject::invokeMethod(this, [this, title, text, details]{
            if (details.isEmpty()) {
                QMessageBox::information(this, title, text);
            } else {
                QMessageBox::information(this, title, text + "\n\n" + details);
            }
        }, Qt::QueuedConnection);
    } else {
        const QString msg = info.trimmed().isEmpty() ? tr("The task failed.") : info.trimmed();
        QMetaObject::invokeMethod(this, [this, msg]{
            QMessageBox::critical(this, tr("Failure"), msg);
        }, Qt::QueuedConnection);
    }

    this->setProperty("lastTask", "");
}

void MainWindow::on_btnSelectImage_clicked()
{
    auto *dlg = createFileDialogWithImagePreview(
        tr("Select Background Image"),
        QString(),
        {"Images (*.png *.jpg *.jpeg *.bmp *.webp *.gif)", "All files (*.*)"}
    );

    if (dlg->exec() != QDialog::Accepted) {
        dlg->deleteLater();
        return;
    }

    const QStringList files = dlg->selectedFiles();
    dlg->deleteLater();

    if (files.isEmpty()) return;

    const QString fn = files.first();
    if (ui->lineImagePath) ui->lineImagePath->setText(fn);
    appendLog("Background image selected: " + fn);
    validateSelectedFilesNonFatal();

    updateLivePreview();
}

void MainWindow::on_btnSelectAnim_clicked()
{
    auto *dlg = createFileDialogWithImagePreview(
        tr("Select Animation"),
        QString(),
        {"Animated (*.gif *.webp *.apng)", "Images (*.png *.jpg *.jpeg *.bmp *.webp *.gif)", "All files (*.*)"}
    );

    if (dlg->exec() != QDialog::Accepted) {
        dlg->deleteLater();
        return;
    }

    const QStringList files = dlg->selectedFiles();
    dlg->deleteLater();

    if (files.isEmpty()) return;

    const QString fn = files.first();
    if (ui->lineAnimPath) ui->lineAnimPath->setText(fn);
    appendLog("Animation selected: " + fn);
    validateSelectedFilesNonFatal();

    updateLivePreview();
}

void MainWindow::on_btnSelectOutDir_clicked()
{
    // Figure out where to start the dialog:
    QString startDir;
    if (ui->lineOutDir && !ui->lineOutDir->text().trimmed().isEmpty()) {
        startDir = ui->lineOutDir->text().trimmed();
    } else {
        startDir = loadDefaultOutDir();
        if (startDir.isEmpty()) startDir = QDir::homePath();
    }

    // Build a directory-only picker dialog (non-native so we can insert our checkbox)
    QFileDialog dlg(this, tr("Select Output Folder"), startDir);
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setOption(QFileDialog::ShowDirsOnly, true);
    dlg.setOption(QFileDialog::DontUseNativeDialog, true);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);

    // "Save as default location" checkbox
    QCheckBox *saveAsDefault = new QCheckBox(tr("Save as default location"), &dlg);
    saveAsDefault->setChecked(false);

    // Add the checkbox to the bottom of the dialog safely
    if (QGridLayout *grid = dlg.findChild<QGridLayout*>()) {
        grid->addWidget(saveAsDefault, grid->rowCount(), 0, 1, grid->columnCount());
    } else if (QLayout *lay = dlg.layout()) {
        // Fallback: attach below existing layout
        QVBoxLayout *wrap = new QVBoxLayout;
        QWidget *container = new QWidget(&dlg);
        container->setLayout(wrap);
        // Move original layout into a container row
        // (If the toolkit layout is not easily movable, just append the checkbox)
        lay->addWidget(saveAsDefault);
    }

    if (dlg.exec() != QDialog::Accepted)
        return;

    const QStringList sel = dlg.selectedFiles();
    if (sel.isEmpty())
        return;

    const QString chosen = sel.first();
    if (chosen.isEmpty())
        return;

    // Put selection into the UI
    if (ui->lineOutDir)
        ui->lineOutDir->setText(chosen);

    // Persist as default if requested
    if (saveAsDefault->isChecked())
        saveDefaultOutDir(chosen);

    // Optional: log / validate if you have helpers for that
    // appendLog("Output folder selected: " + chosen);
    // validateSelectedFilesNonFatal();
}
QString MainWindow::loadDefaultOutDir() const
{
    // Uses app/organization from QCoreApplication if set in main.cpp; otherwise still works.
    QSettings s;
    return s.value(QStringLiteral("paths/defaultOutDir")).toString();
}

void MainWindow::saveDefaultOutDir(const QString &dir)
{
    if (dir.isEmpty())
        return;
    QSettings s;
    s.setValue(QStringLiteral("paths/defaultOutDir"), dir);
    s.sync();
}

void MainWindow::loadDefaultOutDirIntoUi()
{
    const QString def = loadDefaultOutDir();
    if (!def.isEmpty() && ui->lineOutDir && ui->lineOutDir->text().trimmed().isEmpty()) {
        ui->lineOutDir->setText(def);
    }
}


void MainWindow::validateSelectedFilesNonFatal()
{
    const QString bgPath  = ui && ui->lineImagePath ? ui->lineImagePath->text().trimmed() : QString();
    const QString gifPath = ui && ui->lineAnimPath  ? ui->lineAnimPath->text().trimmed()  : QString();

    if (bgPath.isEmpty() && gifPath.isEmpty()) return;

    auto safeProbe = [&](const QString& path, const char* label) -> bool {
        if (path.isEmpty()) { appendLog(QString("[probe] %1: empty").arg(label)); return false; }
        QFileInfo fi(path);
        if (!fi.exists() || !fi.isFile()) {
            appendLog(QString("[probe] %1: not a file: %2").arg(label, path));
            return false;
        }
        QImageReader r(path);
        r.setAutoTransform(true);
        const bool isFile = QFileInfo(path).isFile();
        if (!isFile || !r.canRead()) {
            appendLog(QString("[probe] %1: unreadable (no plugin/format?): %2").arg(label, path));
            return false;
        }
        QImage head = r.read();
        if (head.isNull()) {
            appendLog(QString("[probe] %1: decoder returned null: %2 (err: %3)")
                      .arg(label, path, r.errorString()));
            return false;
        }
        appendLog(QString("[probe] %1 OK: %2  (%3x%4, fmt %5)")
                  .arg(label, fi.fileName())
                  .arg(head.width()).arg(head.height())
                  .arg(QString::fromLatin1(r.format().isEmpty() ? "unknown" : r.format())));
        return true;
    };

    const bool bgOK  = safeProbe(bgPath,  "BG");
    const bool gifOK = safeProbe(gifPath, "ANIM");

    if (bgOK && gifOK) {
        appendLog("[probe] Both selections look good.");
    } else {
        appendLog("[probe] One or both selections are invalid (handled non-fatally).");
    }
}

QString MainWindow::slugify(const QString &s) const
{
    QString out = s.trimmed().toLower();
    out.replace(QRegularExpression("[^a-z0-9]+"), "-");
    out.remove(QRegularExpression("^-+|-+$"));
    if (out.isEmpty()) out = "theme";
    return out;
}

bool MainWindow::copyFileOverwrite(const QString &src, const QString &dst, QString *err)
{
    QFile::remove(dst);
    if (!QFile::copy(src, dst)) {
        if (err) *err = QString("Failed to copy %1 -> %2").arg(src, dst);
        return false;
    }
    return true;
}

bool MainWindow::writeTextFile(const QString &path, const QString &content, QString *err)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (err) *err = "Cannot write: " + path;
        return false;
    }
    if (f.write(content.toUtf8()) == -1) {
        if (err) *err = "Write failed: " + path;
        return false;
    }
    f.close();
    return true;
}

bool MainWindow::runProgram(const QString &program,
                            const QStringList &args,
                            QString *stdoutOut,
                            QString *stderrOut)
{
    QProcess p;
    p.start(program, args);
    if (!p.waitForStarted(60000)) {
        if (stderrOut) *stderrOut = "Failed to start: " + program;
        return false;
    }
    p.closeWriteChannel();
    if (!p.waitForFinished(10 * 60 * 1000)) {
        p.kill();
        if (stderrOut) *stderrOut = "Process timeout: " + program;
        return false;
    }
    if (stdoutOut) *stdoutOut = QString::fromUtf8(p.readAllStandardOutput());
    if (stderrOut) *stderrOut = QString::fromUtf8(p.readAllStandardError());
    return (p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0);
}

bool MainWindow::pruneExtractedFrames(const QString &framesDir, int maxFrames, QString *err)
{
    QStringList files;
    for (QDirIterator it(framesDir, QStringList() << "frame_*.png", QDir::Files); it.hasNext(); )
        files << it.next();
    files.sort();
    const int n = files.size();
    if (n <= maxFrames || maxFrames <= 0) return true;

    const double step = double(n) / double(maxFrames);
    QSet<QString> keep;
    for (int i = 0; i < maxFrames; ++i) {
        int idx = int(i * step + 0.5);
        if (idx >= n) idx = n - 1;
        keep.insert(files[idx]);
    }
    bool ok = true;
    for (const QString &f : std::as_const(files)) {
        if (!keep.contains(f)) {
            if (!QFile::remove(f)) ok = false;
        }
    }
    if (!ok && err) *err = "Some frames could not be removed during pruning.";
    return ok;
}

QString MainWindow::findImageMagick() const
{
    const QString m = QStandardPaths::findExecutable("magick");
    if (!m.isEmpty()) return m;
    const QString c = QStandardPaths::findExecutable("convert");
    if (!c.isEmpty()) return c;
    return {};
}

bool MainWindow::convertImageToPlymouthPng(const QString &src,
                                           const QString &dst,
                                           QString *errOut)
{
    const QString im = findImageMagick();
    if (im.isEmpty()) {
        if (errOut) *errOut = "ImageMagick not found (need 'magick' or 'convert' in PATH).";
        return false;
    }

    QStringList args;
    args << src
         << "-alpha" << "off"
         << "-strip"
         << "-depth" << "8"
         << "-type"  << "TrueColor"
         << "-define" << "png:color-type=2"
         << QString("PNG24:%1").arg(dst);

    QString so, se;
    if (!runProgram(im, args, &so, &se)) {
        if (errOut) *errOut = se.isEmpty() ? QStringLiteral("ImageMagick convert failed.") : se;
        return false;
    }
    return true;
}

bool MainWindow::extractGifFramesPlymouth(const QString &gifPath,
                                          const QString &outDir,
                                          QString *errOut,
                                          int *frameCountOut)
{
    const QString im = findImageMagick();
    if (im.isEmpty()) {
        if (errOut) *errOut = "ImageMagick not found (need 'magick' or 'convert' in PATH).";
        return false;
    }

    QDir().mkpath(outDir);
    for (const QFileInfo &fi : QDir(outDir).entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
        QFile::remove(fi.absoluteFilePath());

    const QString pattern = QDir(outDir).filePath("frame_%04d.png");
    QStringList args;
    args << gifPath
         << "-coalesce"
         << "-background" << "none"
         << "-alpha" << "on"
         << "-strip"
         << "-depth" << "8"
         << "-type"  << "TrueColorMatte"
         << "-define" << "png:color-type=6"
         << QString("PNG32:%1").arg(pattern);

    QString so, se;
    appendLog("Extracting GIF frames → PNG32 (coalesced, transparency preserved) …");
    if (!runProgram(im, args, &so, &se)) {
        if (errOut) *errOut = se.isEmpty() ? QStringLiteral("ImageMagick GIF extract failed.") : se;
        return false;
    }

    QString nerr;
    if (!normalizeFrameFilenames(outDir, &nerr)) {
        if (errOut) *errOut = nerr;
        return false;
    }

    int count = 0;
    for (QDirIterator it(outDir, QStringList() << "frame_*.png", QDir::Files); it.hasNext();) {
        it.next();
        ++count;
    }
    if (frameCountOut) *frameCountOut = count;
    if (count <= 0) {
        if (errOut) *errOut = "No frames produced from GIF.";
        return false;
    }

    appendLog(QString("Frames prepared in %1 (count=%2, with alpha)").arg(outDir).arg(count));
    return true;
}

#include "mainwindow.moc"
