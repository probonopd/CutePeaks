#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    mView        = new TraceView;
    mYSlider     = new QSlider(Qt::Horizontal);
    mXSlider     = new QSlider(Qt::Horizontal);
    mSearchbar   = new QLineEdit();
    mUndoStack   = new QUndoStack(this);

    setCentralWidget(mView);


    SequencePanelWidget * panel = new SequencePanelWidget;

    connect(panel, SIGNAL(selectionChanged(int,int)), mView, SLOT(setSelection(int,int)));

    connect(mSearchbar, &QLineEdit::returnPressed, [this](){

        QRegularExpression exp(mSearchbar->text().toUpper());
        QRegularExpressionMatch m = exp.match(mView->trace()->sequence().byteArray());
        if (m.hasMatch())
            mView->setSelection(m.capturedStart(), m.capturedLength());

    });


    addPanel(panel, Qt::LeftDockWidgetArea);
    addPanel(new InfoPanelWidget, Qt::LeftDockWidgetArea);


    mSearchbar->setMaximumWidth(200);
    mSearchbar->setPlaceholderText(tr("Sequence..."));


    mYSlider->setRange(6,1000);
    mXSlider->setRange(10,1000);
    mXSlider->setToolTip(tr("Scale"));

    mXSlider->setMaximumWidth(100);
    mYSlider->setMaximumWidth(100);
    mYSlider->setValue(0.2);
    mYSlider->setToolTip(tr("Amplitude"));

    QStatusBar * statusBar = new QStatusBar;

    statusBar->addPermanentWidget(new QLabel(tr("Scale")));
    statusBar->addPermanentWidget(mXSlider);
    statusBar->addPermanentWidget(new QLabel(tr("Amplitude")));
    statusBar->addPermanentWidget(mYSlider);
    setStatusBar(statusBar);


    connect(mYSlider, &QSlider::valueChanged, [=](){mView->setAmplitudeFactor(mYSlider->value() / 1000.0 );});
    connect(mXSlider, &QSlider::valueChanged, [=](){mView->setScaleFactor(mXSlider->value() / 100.0);});
    //    connect(mSeqView, &SequenceView::selectionChanged, this, &MainWindow::updateSelection);

    setupActions();

    resize(1000, 400);
    setWindowIcon(QIcon("qrc:/icons/cutepeaks.png"));
    restoreSettings();







}

MainWindow::~MainWindow()
{

}



void MainWindow::openFile()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open file"), QDir::currentPath());
    setFilename(file);

}

void MainWindow::setFilename(const QString &filename)
{
    mFile = filename;

    if (filename.isEmpty()) return;

    if (QFile::exists(filename)){
        mView->setFilename(filename);
        if (mView->isValid())
        {
            for (AbstractPanelWidget * panel : mPanels)
                panel->setTrace(mView->trace());

            QFileInfo info(filename);
            setWindowTitle(info.fileName());
        }
        else
        {
            QMessageBox::critical(this,tr("Error reading"),tr("Cannot read file"));
        }

    }
    else
        QMessageBox::warning(this,tr("Error"),tr("Cannot find file ") + filename);


    //    mView->toPng("/tmp/cutepeaks.png");
    //    mView->toSvg("/tmp/cutepeaks.svg");


}


void MainWindow::closeEvent(QCloseEvent *)
{
    writeSettings();
}

void MainWindow::writeSettings()
{

    QSettings settings;
    settings.setValue("size", rect().size());
    settings.setValue("pos", pos());


}

void MainWindow::restoreSettings()
{

    QSettings settings;
    resize(settings.value("size", QSize(800,400)).toSize());
    move(settings.value("pos").toPoint());

}

void MainWindow::about()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::setTransparent()
{

    setWindowOpacity(windowOpacity() == 1.0 ? 0.3 : 1.0);

}

void MainWindow::removeSelection()
{

    mUndoStack->push(new CutTraceCommand(mView,
                                         mView->currentSelection().pos,
                                         mView->currentSelection().length));



}
void MainWindow::updateSelection()
{

    //    QTextCursor cursor = mSeqView->textCursor();
    //    if (cursor.hasSelection())
    //    {
    //        int start  = cursor.selectionStart();
    //        int length = cursor.selectionEnd() + start;

    //        mView->setSelection(start, length);
    //    }


}

void MainWindow::addPanel(AbstractPanelWidget *panel, Qt::DockWidgetArea area)
{

    mPanels.append(panel);
    QDockWidget * dock = new QDockWidget;
    dock->setWidget(panel);
    dock->setWindowTitle(panel->windowTitle());
    //dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(area,dock);
    //mMetaDock->setVisible(false);
}

void MainWindow::setupActions()
{
    QMenuBar * bar = new QMenuBar;
    setMenuBar(bar);

    // Create app menus
    // file Menu
    QMenu * fileMenu       = bar->addMenu(tr("&File"));
    QAction * openAction   = fileMenu->addAction(tr("&Open"), this, SLOT(openFile()), QKeySequence::Open);
    QAction * saveAction   = fileMenu->addAction(tr("&Save"), this, SLOT(openFile()), QKeySequence::Save);
    QAction * exportAction = fileMenu->addAction(tr("Export As"));
    exportAction->setMenu(new QMenu);
    QAction * exportPng    = exportAction->menu()->addAction(tr("PNG Image"));
    QAction * exportSvg    = exportAction->menu()->addAction(tr("SVG Image"));
    QAction * exportCsv    = exportAction->menu()->addAction(tr("CSV dataset"));
    QAction * exportFasta  = exportAction->menu()->addAction(tr("FASTA sequence"));

    fileMenu->addSeparator();
    fileMenu->addAction(tr("Close"), qApp, SLOT(closeAllWindows()));

    // edit Menu
    QMenu * editMenu       = bar->addMenu(tr("&Edit"));

    editMenu->addAction(mUndoStack->createUndoAction(this,"Undo"));
    editMenu->addAction(mUndoStack->createRedoAction(this,"Redo"));
    editMenu->addSeparator();
    editMenu->addAction(tr("Copy base(s)"), this,SLOT(openFile()),QKeySequence::Copy);
    editMenu->addAction(tr("Copy amino acid(s)"), this,SLOT(openFile()));
    editMenu->addSeparator();
    editMenu->addAction(tr("Select all"), this,SLOT(openFile()), QKeySequence::SelectAll);
    editMenu->addSeparator();
    QAction * remAction = editMenu->addAction(tr("Remove selection"), this,SLOT(openFile()),QKeySequence::Delete);
    QAction * revAction = editMenu->addAction(tr("Revert Sequence"), mView,SLOT(revert()),  QKeySequence(Qt::CTRL + Qt::Key_I));
    editMenu->addSeparator();
    editMenu->addAction(tr("Find Sequence"), this,SLOT(openFile()),  QKeySequence::Find);


    // view Menu
    QMenu * viewMenu          = bar->addMenu(tr("&View"));

    QAction * showQualAction     = viewMenu->addAction(tr("Show quality"), this, SLOT(openFile()));
    QAction * showAminoAction    = viewMenu->addAction(tr("Show aminoacid"), this, SLOT(openFile()));
    viewMenu->addSeparator();
    QAction * showSequenceAction = viewMenu->addAction(tr("Show sequence"), this, SLOT(openFile()));
    QAction * showMetadataAction = viewMenu->addAction(tr("Show metadata"), this, SLOT(openFile()));


    showQualAction->setCheckable(true);
    showAminoAction->setCheckable(true);
    showSequenceAction->setCheckable(true);
    showMetadataAction->setCheckable(true);
    viewMenu->addSeparator();
    QAction * aminoAcidAction = viewMenu->addAction(tr("frameshift"));
    aminoAcidAction->setMenu(new QMenu());
    QActionGroup * frameGroup = new QActionGroup(this);
    frameGroup->addAction(aminoAcidAction->menu()->addAction("frame 1",[this](){mView->setFrameShift(TraceView::Shift_1);}));
    frameGroup->addAction(aminoAcidAction->menu()->addAction("frame 2",[this](){mView->setFrameShift(TraceView::Shift_2);}));
    frameGroup->addAction(aminoAcidAction->menu()->addAction("frame 3",[this](){mView->setFrameShift(TraceView::Shift_3);}));

    frameGroup->setExclusive(true);
    for (auto a : frameGroup->actions())
        a->setCheckable(true);

    frameGroup->actions().first()->setChecked(true);
    viewMenu->addSeparator();

    QMenu * helpMenu = bar->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, SLOT(about()));
    helpMenu->addAction(tr("About Qt"), qApp, SLOT(aboutQt()));

    QToolBar * toolbar = addToolBar("mainbar");
    toolbar->addAction(openAction);
    toolbar->addAction(saveAction);
    toolbar->addAction(exportAction);
    toolbar->addSeparator();
    toolbar->addAction(remAction);
    toolbar->addAction(revAction);
    toolbar->addSeparator();
    toolbar->addAction(aminoAcidAction);



    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(spacer);

    toolbar->addWidget(mSearchbar);



}



void MainWindow::keyPressEvent(QKeyEvent *event)
{

    QSvgGenerator generator;
    generator.setFileName("/tmp/capture.svg");
    generator.setTitle("test");
    generator.setDescription("description");
    render(&generator);


    return QMainWindow::keyPressEvent(event);

}
