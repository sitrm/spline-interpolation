#ifndef WIDGET_H
#define WIDGET_H
//------------------------------------
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QtGui>
#include <QApplication>
//-------------------------------------
#include "ippCustom.h"
#include "spline.h"
//--------------------------------------
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_input_PBN_clicked();
    void on_output_PBN_clicked();
    void on_start_PBN_clicked();

private:
    static const int    BUF_SIZE = 100000;
private:
    Ui::Widget          *ui;

    QString             pathFile_input;
    QString             pathFile_output;

    Ipp32fc             *signal_buf;
    Ipp32fc             *signal_buf_c;
    double              *time_buf;
    int                 Fold_save, Fnew_save;

    void                log( const QString &s);


};
#endif // WIDGET_H
