#pragma once

#include <QMainWindow>
#include <QString>
#include <qfiledialog.h>

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
    Ui::MainWindow *ui;


    // ---- (Optional) helper declarations used by your generator/installer ----
    // If your project already declares these elsewhere, you can delete these lines.
    QString slugify(const QString &s) const;
    bool copyFileOverwrite(const QString &src, const QString &dst, QString *err);
    bool writeTextFile(const QString &path, const QString &content, QString *err);
    bool runProgram(const QString &program, const QStringList &args, QString *stdoutOut, QString *stderrOut);
    bool pruneExtractedFrames(const QString &framesDir, int maxFrames, QString *err);
    QString findImageMagick() const;
};
