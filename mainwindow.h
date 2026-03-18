#pragma once

#include <QMainWindow>
#include <QString>
#include <qfiledialog.h>
#include <QSpinBox>
#include <QSignalBlocker>
#include <QGroupBox>
#include <QImage>
#include <QMovie>
#include <QPointer>
#include <QSize>
#include <QPoint>
#include <QMouseEvent>
#include <QLabel>

class PreviewDragLabel : public QLabel
{
    Q_OBJECT
public:
    explicit PreviewDragLabel(QWidget *parent = nullptr) : QLabel(parent) {}

signals:
    void mousePressed(const QPoint &pos);
    void mouseMoved(const QPoint &pos, Qt::MouseButtons buttons);
    void mouseReleased(const QPoint &pos);

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        emit mousePressed(event->pos());
        QLabel::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        emit mouseMoved(event->pos(), event->buttons());
        QLabel::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        emit mouseReleased(event->pos());
        QLabel::mouseReleaseEvent(event);
    }
};

namespace Ui { class MainWindow; }   // ensure this matches the .ui class name

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool runRootScriptStreaming(const QString &scriptContent,
                           QString *errOut);

    // ---- Thread-safe UI helpers (implemented in mainwindow.cpp) ----
    void appendLog(const QString &msg);
    void setUiBusy(bool busy);
    bool installThemePrivilegedCore(const QString &themeName,
                                    const QString &themeDir,
                                    QString *errOut);
    QString generatePlymouthThemeFilesCore(const QString &themeName,
                                           const QString &bgImagePath,
                                           const QString &outBaseDir,
                                           bool /*scaleToScreen*/,
                                           const QString &animGifPath,
                                           QString *errOut);

void ensureGlobalIndicatorStyle();

    // ---- Background-safe wrappers (call your existing originals under the hood) ----
    // Returns themeDir on success; empty string on failure (and *err set).
    QString generateThemeFilesBg(const QString &themeName,
                                 const QString &bgImagePath,
                                 const QString &outBaseDir,
                                 bool scaleToScreen,
                                 const QString &animGifPath,
                                 QString *err);
    // Returns true on success; false on failure (and *err set).
    bool installThemePrivilegedBg(const QString &themeName,
                                  const QString &themeDir,
                                  QString *err);

    // ---- Starters that spin up the QThread worker ----
    void startGenerateInBackground();
    void startGenerateAndInstallInBackground();

    // ---- Your existing originals (already in your project) ----
    // NOTE: these are the ones you previously had; the Bg versions call these.
    QString generatePlymouthThemeFiles(const QString &themeName,
                                       const QString &bgImagePath,
                                       const QString &outBaseDir,
                                       bool scaleToScreen,
                                       const QString &animGifPath);
    bool installThemePrivileged(const QString &themeName,
                                const QString &themeDir,
                                QString *err);

    bool installThemePrivilegedCoreStreaming(const QString &themeName,
                                               const QString &themeDir,
                                             QString *errOut);
    bool convertImageToPlymouthPng(const QString &src,
                                           const QString &dst,
                                           QString *errOut);


    bool extractGifFramesPlymouth(const QString &gifPath,
                                              const QString &outDir,
                                              QString *errOut,
                                              int *frameCountOut);
    bool createPreviewPngFromFrames(const QString& themeDir,
                                 const QString& backgroundPath,
                                 QString* outErrorMessage /*=nullptr*/);

    QFileDialog* createFileDialogWithImagePreview(const QString& title,
                                              const QString& startDir,
                                              const QStringList& nameFilters);

    void validateSelectedFilesNonFatal();

    bool resizeAnimationFrames(const QString &framesDir,
                          int targetWidth,
                          int targetHeight,
                          bool keepAspect,
                          QString *errOut);

    void ensureSingleClickSelectOnly(QFileDialog *dlg);

    bool generatePreviewAfterResize(const QString &bgPath,
                                            const QString &framesDir,
                                            const QString &outDir,
                                            QString *errOut);

    bool    refreshPreviewFromCurrentUI(const QString &framesDir, QString *errOut);

    void showPreviewPopup(const QString &pngPath);
private slots:
    void onInstalledThemeChanged(const QString &themeName);
    void showInstalledThemeContextMenu(const QPoint &pos);

    // If your buttons are connected in Designer or via auto-connect, keep these:
    void on_btnGenerate_clicked();
    void on_btnInstall_clicked();
    void on_btnSelectAnim_clicked();
    void on_btnSelectOutDir_clicked();
    void on_btnSelectImage_clicked();
    // Worker thread signal handlers (defined in mainwindow.cpp)
    void onWorkerProgress(const QString &line);
    void onWorkerError(const QString &line);
    void onWorkerFinished(bool ok, const QString &info);

private:
    void setupPreviewMousePositioning();
    bool computePreviewStageRect(QRect *stageRectOut, QSize *virtualScreenOut) const;
    bool currentRenderedAnimationRectInStage(QRect *animRectOut, QRect *stageRectOut = nullptr) const;
    void setAnimationPercentFromPreviewPoint(const QPoint &previewPos, bool clampToBounds = true);

    QString detectCurrentSystemTheme() const;
    QString displayNameForInstalledTheme(const QString &themeName) const;

    bool m_previewDragging = false;
    QPoint m_dragHotspot;

    void refreshInstalledThemes();
    QString installedThemesRoot() const;
    QString installedThemePreviewPath(const QString &themeName) const;
    bool activateInstalledThemePrivileged(const QString &themeName, QString *errOut);
    bool deleteInstalledThemePrivileged(const QString &themeName, QString *errOut);

    bool m_loadingInstalledThemes = false;

    QString m_livePreviewFramesDir;

    void setupLivePreviewConnections();
    void updateLivePreview();              // keep this name so existing calls still work
    void updateLivePreviewSources();
    void renderLivePreview();
    QSize effectiveAnimationPreviewSize(const QSize &srcSize) const;
    bool rebuildLivePreviewFrames(QString *errOut);

    QImage m_previewBackground;
    QPointer<QMovie> m_previewMovie;
    QString m_previewMoviePath;

    Ui::MainWindow *ui;
    QString loadDefaultOutDir() const;
    void saveDefaultOutDir(const QString &dir);
    void loadDefaultOutDirIntoUi();

    // ---- (Optional) helper declarations used by your generator/installer ----
    // If your project already declares these elsewhere, you can delete these lines.
    QString slugify(const QString &s) const;
    bool copyFileOverwrite(const QString &src, const QString &dst, QString *err);
    bool writeTextFile(const QString &path, const QString &content, QString *err);
    bool runProgram(const QString &program, const QStringList &args, QString *stdoutOut, QString *stderrOut);
    bool pruneExtractedFrames(const QString &framesDir, int maxFrames, QString *err);
    QString findImageMagick() const;
};
