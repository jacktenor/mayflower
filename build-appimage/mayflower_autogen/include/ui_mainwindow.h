/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.2.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *themeNameLayout;
    QLabel *labelTheme;
    QLineEdit *lineThemeName;
    QGroupBox *groupFiles;
    QGridLayout *gridFiles;
    QLabel *labelBg;
    QLineEdit *lineImagePath;
    QPushButton *btnSelectImage;
    QLabel *labelGif;
    QLineEdit *lineAnimPath;
    QPushButton *btnSelectAnim;
    QLabel *labelOut;
    QLineEdit *lineOutDir;
    QPushButton *btnSelectOutDir;
    QHBoxLayout *settingsLayout;
    QGroupBox *groupOptions;
    QVBoxLayout *optionsLayout;
    QLabel *labelResize;
    QHBoxLayout *widthLayout;
    QLabel *labelW;
    QSpinBox *spinAnimW;
    QHBoxLayout *heightLayout;
    QLabel *labelH;
    QSpinBox *spinAnimH;
    QCheckBox *chkKeepAspect;
    QFrame *separator1;
    QCheckBox *chkScaleToScreen;
    QSpacerItem *optionsSpacer;
    QGroupBox *groupPlacement;
    QVBoxLayout *placementLayout;
    QHBoxLayout *positionPresetLayout;
    QLabel *labelPosition;
    QComboBox *comboAnimPlacement;
    QFrame *separator2;
    QCheckBox *usePercentPlacement;
    QHBoxLayout *xPercentLayout;
    QLabel *labelXPercent;
    QSpinBox *xPercentSpin;
    QHBoxLayout *yPercentLayout;
    QLabel *labelYPercent;
    QSpinBox *yPercentSpin;
    QFrame *separator3;
    QHBoxLayout *fpsLayout;
    QLabel *labelFPS;
    QSpinBox *fpsSpin;
    QSpacerItem *placementSpacer;
    QGroupBox *groupLog;
    QVBoxLayout *logLayout;
    QPlainTextEdit *textLog;
    QHBoxLayout *buttonLayout;
    QSpacerItem *buttonSpacer;
    QPushButton *btnGenerate;
    QPushButton *btnInstall;
    QLabel *label;
    QMenuBar *menubar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 800);
        MainWindow->setMinimumSize(QSize(800, 800));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        mainLayout = new QVBoxLayout(centralwidget);
        mainLayout->setSpacing(12);
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        mainLayout->setContentsMargins(16, 16, 16, 16);
        themeNameLayout = new QHBoxLayout();
        themeNameLayout->setObjectName(QString::fromUtf8("themeNameLayout"));
        labelTheme = new QLabel(centralwidget);
        labelTheme->setObjectName(QString::fromUtf8("labelTheme"));

        themeNameLayout->addWidget(labelTheme);

        lineThemeName = new QLineEdit(centralwidget);
        lineThemeName->setObjectName(QString::fromUtf8("lineThemeName"));

        themeNameLayout->addWidget(lineThemeName);


        mainLayout->addLayout(themeNameLayout);

        groupFiles = new QGroupBox(centralwidget);
        groupFiles->setObjectName(QString::fromUtf8("groupFiles"));
        gridFiles = new QGridLayout(groupFiles);
        gridFiles->setObjectName(QString::fromUtf8("gridFiles"));
        gridFiles->setHorizontalSpacing(12);
        gridFiles->setVerticalSpacing(10);
        labelBg = new QLabel(groupFiles);
        labelBg->setObjectName(QString::fromUtf8("labelBg"));

        gridFiles->addWidget(labelBg, 0, 0, 1, 1);

        lineImagePath = new QLineEdit(groupFiles);
        lineImagePath->setObjectName(QString::fromUtf8("lineImagePath"));

        gridFiles->addWidget(lineImagePath, 0, 1, 1, 1);

        btnSelectImage = new QPushButton(groupFiles);
        btnSelectImage->setObjectName(QString::fromUtf8("btnSelectImage"));

        gridFiles->addWidget(btnSelectImage, 0, 2, 1, 1);

        labelGif = new QLabel(groupFiles);
        labelGif->setObjectName(QString::fromUtf8("labelGif"));

        gridFiles->addWidget(labelGif, 1, 0, 1, 1);

        lineAnimPath = new QLineEdit(groupFiles);
        lineAnimPath->setObjectName(QString::fromUtf8("lineAnimPath"));

        gridFiles->addWidget(lineAnimPath, 1, 1, 1, 1);

        btnSelectAnim = new QPushButton(groupFiles);
        btnSelectAnim->setObjectName(QString::fromUtf8("btnSelectAnim"));

        gridFiles->addWidget(btnSelectAnim, 1, 2, 1, 1);

        labelOut = new QLabel(groupFiles);
        labelOut->setObjectName(QString::fromUtf8("labelOut"));

        gridFiles->addWidget(labelOut, 2, 0, 1, 1);

        lineOutDir = new QLineEdit(groupFiles);
        lineOutDir->setObjectName(QString::fromUtf8("lineOutDir"));

        gridFiles->addWidget(lineOutDir, 2, 1, 1, 1);

        btnSelectOutDir = new QPushButton(groupFiles);
        btnSelectOutDir->setObjectName(QString::fromUtf8("btnSelectOutDir"));

        gridFiles->addWidget(btnSelectOutDir, 2, 2, 1, 1);


        mainLayout->addWidget(groupFiles);

        settingsLayout = new QHBoxLayout();
        settingsLayout->setSpacing(12);
        settingsLayout->setObjectName(QString::fromUtf8("settingsLayout"));
        groupOptions = new QGroupBox(centralwidget);
        groupOptions->setObjectName(QString::fromUtf8("groupOptions"));
        optionsLayout = new QVBoxLayout(groupOptions);
        optionsLayout->setSpacing(10);
        optionsLayout->setObjectName(QString::fromUtf8("optionsLayout"));
        labelResize = new QLabel(groupOptions);
        labelResize->setObjectName(QString::fromUtf8("labelResize"));
        QFont font;
        font.setBold(true);
        labelResize->setFont(font);

        optionsLayout->addWidget(labelResize);

        widthLayout = new QHBoxLayout();
        widthLayout->setObjectName(QString::fromUtf8("widthLayout"));
        labelW = new QLabel(groupOptions);
        labelW->setObjectName(QString::fromUtf8("labelW"));

        widthLayout->addWidget(labelW);

        spinAnimW = new QSpinBox(groupOptions);
        spinAnimW->setObjectName(QString::fromUtf8("spinAnimW"));
        spinAnimW->setMinimum(0);
        spinAnimW->setMaximum(16384);
        spinAnimW->setValue(0);

        widthLayout->addWidget(spinAnimW);


        optionsLayout->addLayout(widthLayout);

        heightLayout = new QHBoxLayout();
        heightLayout->setObjectName(QString::fromUtf8("heightLayout"));
        labelH = new QLabel(groupOptions);
        labelH->setObjectName(QString::fromUtf8("labelH"));

        heightLayout->addWidget(labelH);

        spinAnimH = new QSpinBox(groupOptions);
        spinAnimH->setObjectName(QString::fromUtf8("spinAnimH"));
        spinAnimH->setMinimum(0);
        spinAnimH->setMaximum(16384);
        spinAnimH->setValue(0);

        heightLayout->addWidget(spinAnimH);


        optionsLayout->addLayout(heightLayout);

        chkKeepAspect = new QCheckBox(groupOptions);
        chkKeepAspect->setObjectName(QString::fromUtf8("chkKeepAspect"));
        chkKeepAspect->setChecked(true);

        optionsLayout->addWidget(chkKeepAspect);

        separator1 = new QFrame(groupOptions);
        separator1->setObjectName(QString::fromUtf8("separator1"));
        separator1->setFrameShape(QFrame::HLine);
        separator1->setFrameShadow(QFrame::Sunken);

        optionsLayout->addWidget(separator1);

        chkScaleToScreen = new QCheckBox(groupOptions);
        chkScaleToScreen->setObjectName(QString::fromUtf8("chkScaleToScreen"));
        chkScaleToScreen->setChecked(true);

        optionsLayout->addWidget(chkScaleToScreen);

        optionsSpacer = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);

        optionsLayout->addItem(optionsSpacer);


        settingsLayout->addWidget(groupOptions);

        groupPlacement = new QGroupBox(centralwidget);
        groupPlacement->setObjectName(QString::fromUtf8("groupPlacement"));
        placementLayout = new QVBoxLayout(groupPlacement);
        placementLayout->setSpacing(10);
        placementLayout->setObjectName(QString::fromUtf8("placementLayout"));
        positionPresetLayout = new QHBoxLayout();
        positionPresetLayout->setObjectName(QString::fromUtf8("positionPresetLayout"));
        labelPosition = new QLabel(groupPlacement);
        labelPosition->setObjectName(QString::fromUtf8("labelPosition"));

        positionPresetLayout->addWidget(labelPosition);

        comboAnimPlacement = new QComboBox(groupPlacement);
        comboAnimPlacement->addItem(QString());
        comboAnimPlacement->addItem(QString());
        comboAnimPlacement->addItem(QString());
        comboAnimPlacement->addItem(QString());
        comboAnimPlacement->addItem(QString());
        comboAnimPlacement->addItem(QString());
        comboAnimPlacement->addItem(QString());
        comboAnimPlacement->addItem(QString());
        comboAnimPlacement->addItem(QString());
        comboAnimPlacement->setObjectName(QString::fromUtf8("comboAnimPlacement"));

        positionPresetLayout->addWidget(comboAnimPlacement);


        placementLayout->addLayout(positionPresetLayout);

        separator2 = new QFrame(groupPlacement);
        separator2->setObjectName(QString::fromUtf8("separator2"));
        separator2->setFrameShape(QFrame::HLine);
        separator2->setFrameShadow(QFrame::Sunken);

        placementLayout->addWidget(separator2);

        usePercentPlacement = new QCheckBox(groupPlacement);
        usePercentPlacement->setObjectName(QString::fromUtf8("usePercentPlacement"));
        usePercentPlacement->setChecked(false);

        placementLayout->addWidget(usePercentPlacement);

        xPercentLayout = new QHBoxLayout();
        xPercentLayout->setObjectName(QString::fromUtf8("xPercentLayout"));
        labelXPercent = new QLabel(groupPlacement);
        labelXPercent->setObjectName(QString::fromUtf8("labelXPercent"));

        xPercentLayout->addWidget(labelXPercent);

        xPercentSpin = new QSpinBox(groupPlacement);
        xPercentSpin->setObjectName(QString::fromUtf8("xPercentSpin"));
        xPercentSpin->setEnabled(false);
        xPercentSpin->setMinimum(0);
        xPercentSpin->setMaximum(100);
        xPercentSpin->setValue(50);

        xPercentLayout->addWidget(xPercentSpin);


        placementLayout->addLayout(xPercentLayout);

        yPercentLayout = new QHBoxLayout();
        yPercentLayout->setObjectName(QString::fromUtf8("yPercentLayout"));
        labelYPercent = new QLabel(groupPlacement);
        labelYPercent->setObjectName(QString::fromUtf8("labelYPercent"));

        yPercentLayout->addWidget(labelYPercent);

        yPercentSpin = new QSpinBox(groupPlacement);
        yPercentSpin->setObjectName(QString::fromUtf8("yPercentSpin"));
        yPercentSpin->setEnabled(false);
        yPercentSpin->setMinimum(0);
        yPercentSpin->setMaximum(100);
        yPercentSpin->setValue(50);

        yPercentLayout->addWidget(yPercentSpin);


        placementLayout->addLayout(yPercentLayout);

        separator3 = new QFrame(groupPlacement);
        separator3->setObjectName(QString::fromUtf8("separator3"));
        separator3->setFrameShape(QFrame::HLine);
        separator3->setFrameShadow(QFrame::Sunken);

        placementLayout->addWidget(separator3);

        fpsLayout = new QHBoxLayout();
        fpsLayout->setObjectName(QString::fromUtf8("fpsLayout"));
        labelFPS = new QLabel(groupPlacement);
        labelFPS->setObjectName(QString::fromUtf8("labelFPS"));

        fpsLayout->addWidget(labelFPS);

        fpsSpin = new QSpinBox(groupPlacement);
        fpsSpin->setObjectName(QString::fromUtf8("fpsSpin"));
        fpsSpin->setMinimum(1);
        fpsSpin->setMaximum(60);
        fpsSpin->setValue(16);

        fpsLayout->addWidget(fpsSpin);


        placementLayout->addLayout(fpsLayout);

        placementSpacer = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);

        placementLayout->addItem(placementSpacer);


        settingsLayout->addWidget(groupPlacement);


        mainLayout->addLayout(settingsLayout);

        groupLog = new QGroupBox(centralwidget);
        groupLog->setObjectName(QString::fromUtf8("groupLog"));
        logLayout = new QVBoxLayout(groupLog);
        logLayout->setObjectName(QString::fromUtf8("logLayout"));
        textLog = new QPlainTextEdit(groupLog);
        textLog->setObjectName(QString::fromUtf8("textLog"));
        textLog->setReadOnly(true);

        logLayout->addWidget(textLog);


        mainLayout->addWidget(groupLog);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        buttonSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(buttonSpacer);

        btnGenerate = new QPushButton(centralwidget);
        btnGenerate->setObjectName(QString::fromUtf8("btnGenerate"));
        btnGenerate->setMinimumSize(QSize(140, 32));

        buttonLayout->addWidget(btnGenerate);

        btnInstall = new QPushButton(centralwidget);
        btnInstall->setObjectName(QString::fromUtf8("btnInstall"));
        btnInstall->setMinimumSize(QSize(140, 32));

        buttonLayout->addWidget(btnInstall);


        mainLayout->addLayout(buttonLayout);

        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label->setOpenExternalLinks(true);

        mainLayout->addWidget(label);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 27));
        MainWindow->setMenuBar(menubar);

        retranslateUi(MainWindow);
        QObject::connect(usePercentPlacement, &QCheckBox::toggled, xPercentSpin, &QSpinBox::setEnabled);
        QObject::connect(usePercentPlacement, &QCheckBox::toggled, yPercentSpin, &QSpinBox::setEnabled);

        comboAnimPlacement->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Mayflower - Boot Splash Creator", nullptr));
        labelTheme->setText(QCoreApplication::translate("MainWindow", "Theme Name:", nullptr));
        lineThemeName->setPlaceholderText(QCoreApplication::translate("MainWindow", "Enter your theme name...", nullptr));
        groupFiles->setTitle(QCoreApplication::translate("MainWindow", "Files", nullptr));
        labelBg->setText(QCoreApplication::translate("MainWindow", "Background Image:", nullptr));
        lineImagePath->setPlaceholderText(QCoreApplication::translate("MainWindow", "Select a background image...", nullptr));
#if QT_CONFIG(tooltip)
        btnSelectImage->setToolTip(QCoreApplication::translate("MainWindow", "Choose background image", nullptr));
#endif // QT_CONFIG(tooltip)
        btnSelectImage->setText(QCoreApplication::translate("MainWindow", "Browse...", nullptr));
        labelGif->setText(QCoreApplication::translate("MainWindow", "Animation GIF:", nullptr));
        lineAnimPath->setPlaceholderText(QCoreApplication::translate("MainWindow", "Select an animation GIF...", nullptr));
#if QT_CONFIG(tooltip)
        btnSelectAnim->setToolTip(QCoreApplication::translate("MainWindow", "Choose animation GIF", nullptr));
#endif // QT_CONFIG(tooltip)
        btnSelectAnim->setText(QCoreApplication::translate("MainWindow", "Browse...", nullptr));
        labelOut->setText(QCoreApplication::translate("MainWindow", "Output Directory:", nullptr));
        lineOutDir->setPlaceholderText(QCoreApplication::translate("MainWindow", "Select an output directory...", nullptr));
#if QT_CONFIG(tooltip)
        btnSelectOutDir->setToolTip(QCoreApplication::translate("MainWindow", "Choose output folder", nullptr));
#endif // QT_CONFIG(tooltip)
        btnSelectOutDir->setText(QCoreApplication::translate("MainWindow", "Browse...", nullptr));
        groupOptions->setTitle(QCoreApplication::translate("MainWindow", "Animation Options", nullptr));
        labelResize->setText(QCoreApplication::translate("MainWindow", "Resize Animation", nullptr));
        labelW->setText(QCoreApplication::translate("MainWindow", "Width:", nullptr));
#if QT_CONFIG(tooltip)
        spinAnimW->setToolTip(QCoreApplication::translate("MainWindow", "Target width (0 = auto)", nullptr));
#endif // QT_CONFIG(tooltip)
        spinAnimW->setSpecialValueText(QCoreApplication::translate("MainWindow", "auto", nullptr));
        spinAnimW->setSuffix(QCoreApplication::translate("MainWindow", " px", nullptr));
        labelH->setText(QCoreApplication::translate("MainWindow", "Height:", nullptr));
#if QT_CONFIG(tooltip)
        spinAnimH->setToolTip(QCoreApplication::translate("MainWindow", "Target height (0 = auto)", nullptr));
#endif // QT_CONFIG(tooltip)
        spinAnimH->setSpecialValueText(QCoreApplication::translate("MainWindow", "auto", nullptr));
        spinAnimH->setSuffix(QCoreApplication::translate("MainWindow", " px", nullptr));
        chkKeepAspect->setText(QCoreApplication::translate("MainWindow", "Keep aspect ratio", nullptr));
#if QT_CONFIG(tooltip)
        chkScaleToScreen->setToolTip(QCoreApplication::translate("MainWindow", "When enabled, background is scaled to fit screen", nullptr));
#endif // QT_CONFIG(tooltip)
        chkScaleToScreen->setText(QCoreApplication::translate("MainWindow", "Scale background to screen", nullptr));
        groupPlacement->setTitle(QCoreApplication::translate("MainWindow", "Animation Placement", nullptr));
        labelPosition->setText(QCoreApplication::translate("MainWindow", "Position:", nullptr));
        comboAnimPlacement->setItemText(0, QCoreApplication::translate("MainWindow", "center", nullptr));
        comboAnimPlacement->setItemText(1, QCoreApplication::translate("MainWindow", "bottom-center", nullptr));
        comboAnimPlacement->setItemText(2, QCoreApplication::translate("MainWindow", "top-center", nullptr));
        comboAnimPlacement->setItemText(3, QCoreApplication::translate("MainWindow", "left-center", nullptr));
        comboAnimPlacement->setItemText(4, QCoreApplication::translate("MainWindow", "right-center", nullptr));
        comboAnimPlacement->setItemText(5, QCoreApplication::translate("MainWindow", "bottom-left", nullptr));
        comboAnimPlacement->setItemText(6, QCoreApplication::translate("MainWindow", "bottom-right", nullptr));
        comboAnimPlacement->setItemText(7, QCoreApplication::translate("MainWindow", "top-left", nullptr));
        comboAnimPlacement->setItemText(8, QCoreApplication::translate("MainWindow", "top-right", nullptr));

        usePercentPlacement->setText(QCoreApplication::translate("MainWindow", "Use custom percentage placement", nullptr));
        labelXPercent->setText(QCoreApplication::translate("MainWindow", "X Position:", nullptr));
        xPercentSpin->setSuffix(QCoreApplication::translate("MainWindow", " %", nullptr));
        labelYPercent->setText(QCoreApplication::translate("MainWindow", "Y Position:", nullptr));
        yPercentSpin->setSuffix(QCoreApplication::translate("MainWindow", " %", nullptr));
        labelFPS->setText(QCoreApplication::translate("MainWindow", "Frame Rate:", nullptr));
        fpsSpin->setSuffix(QCoreApplication::translate("MainWindow", " fps", nullptr));
        groupLog->setTitle(QCoreApplication::translate("MainWindow", "Output Log", nullptr));
        textLog->setPlaceholderText(QCoreApplication::translate("MainWindow", "Log output will appear here...", nullptr));
#if QT_CONFIG(tooltip)
        btnGenerate->setToolTip(QCoreApplication::translate("MainWindow", "Generate theme files", nullptr));
#endif // QT_CONFIG(tooltip)
        btnGenerate->setText(QCoreApplication::translate("MainWindow", "Generate Theme", nullptr));
#if QT_CONFIG(tooltip)
        btnInstall->setToolTip(QCoreApplication::translate("MainWindow", "Install the generated theme", nullptr));
#endif // QT_CONFIG(tooltip)
        btnInstall->setText(QCoreApplication::translate("MainWindow", "Install Theme", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p><a href=\"https://www.beeralator.com\"><span style=\" text-decoration: underline; color:#0000ff;\">Beeralator.com</span></a></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
