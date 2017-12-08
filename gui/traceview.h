#ifndef TESTVIEW_H
#define TESTVIEW_H
#include <QtWidgets>
#include <QScroller>
#include <QSvgGenerator>
#include <QPropertyAnimation>
#include "tracefactory.h"
#include "abifsequencetrace.h"
#include "tracecolor.h"


struct Selection
{
    int pos;
    int length;
};

class TraceView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    TraceView(QWidget * parent = 0);
    /*!
     * \brief setFilename
     * \param filename
     * Load a trace file
     */
    void setFilename(const QString& filename);

    void setTrace(Trace * trace);

    /*!
     * \brief trace
     * \return the trace sequence object
     */
    const Trace * trace() const;

    /*!
     * \brief isValid
     * \return true is the sequence is valid and has been loaded
     */
    bool isValid() const;

    /*!
     * \brief toSvg
     * \param filename
     * save current view as svg
     * \return true if success
     */
    bool toSvg(const QString& filename);
    /*!
     * \brief toPng
     * \param filename
     * save current view as png
     * \return true if success
     */
    bool toPng(const QString& filename);

    /*!
     * \brief traceToView
     * \param x
     * Convert trace coordinate to view coordinate
     * \return
     */
    int traceToView(int x);

    void scrollTo(int pos, bool animate = true);


public Q_SLOTS:
    void setAmplitudeFactor(float factor);
    void setScaleFactor(float factor);
    void setSelection(int pos, int length = 1);

protected :
    void paintEvent(QPaintEvent * event)  override;
    void scrollContentsBy(int dx, int dy) override;
    void resizeEvent(QResizeEvent * event)override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    bool viewportEvent(QEvent * event) override;
    void setupViewport();



    /*!
     * \brief inView
     * \param pos
     * \param margin
     * \return true if the position is in the viewport according margin
     */
    bool inView(int pos, int margin = 0) const;

    void updateScrollbar();

    void drawAll(QPainter& painter);

    /*!
     * \brief drawConfident
     * \param painter
     * Draw confident score as a rect blue path
     */
    void drawConfident(QPainter& painter);
    /*!
     * \brief drawBases
     * \param painter
     * draw nucleotid text on the top of the trace
     */
    void drawBases(QPainter& painter);
    /*!
     * \brief drawAminoAcid
     * \param painter
     * draw amino acid text on the top of the trace
     */
    void drawAminoAcid(QPainter& painter);
    /*!
     * \brief drawTraces
     * \param painter
     * Draw all traces from AbstractSequenceTrace
     */
    void drawTraces(QPainter& painter);
    /*!
     * \brief drawSelection
     * \param painter
     * Draw selection
     */
    void drawSelection(QPainter& painter);


private:
    QString mFilename;
    Trace * mTrace;


    // data
    QHash< QString, QVector<QPointF>> mLineSeries;

    // scaling
    float mYFactor = 0.1;
    float mXFactor = 2;

    // scrolling
    int mXStart = 0;

    // qc bar
    float mQCHeightFactor = 1.4;
    int mQCWidth = 10;

    // trace colors
    QScroller * mScroller;

    Selection mCurrentSelection = { 20, 10};

    QPropertyAnimation * mScrollAnimation;


};

#endif // TESTVIEW_H
