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
    static const int BUF_SIZE = 1000;
private:
    Ui::Widget          *ui;
    QString             fileName;
    QString             pathFile_input;
    QString             pathFile_output;
    Ipp32fc             *signal_buf;
    int                 Fold_save, Fnew_save;

    void log( const QString &s);
    bool                flag_input=false;
    bool                flag_output=false;

};
#endif // WIDGET_H
