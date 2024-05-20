    #include "widget.h"
#include "ui_widget.h"
//-----------------------------------------------------------
#include <cstdio>
#include <vector>
#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>

//-----------------------------------------------------------
#include "ippCustom.h"
//-----------------------------------------------------------
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    QSettings s;

    this->restoreGeometry(s.value("geometry").toByteArray());
    //сохранили пути на кнопках
    pathFile_input = s.value( "pathIn", QDir::homePath() ).toString();
    pathFile_output = s.value( "pathOut", QDir::homePath() ).toString();
    Fold_save = s.value("Fold_save", 44100).toInt();
    Fnew_save = s.value("Fnew_save", 22050).toInt();

    ui->input_LED->setText(QDir::toNativeSeparators(pathFile_input));
    ui->output_LED->setText(QDir::toNativeSeparators(pathFile_output));

    ui->FdNew_SBX->setValue(Fnew_save);
    ui->FdOld_SBX->setValue(Fold_save);

    //если уже есть какие то пути, то сразу же разрешаем start
    if(!ui->input_LED->text().isEmpty()   // проверяем еще на пустоту строк в LineEdit
       && !ui->output_LED->text().isEmpty()){
        ui->start_PBN->setEnabled(true);
    }

    signal_buf = new Ipp32fc [BUF_SIZE];

    log( "Приложение запущено" );


}
//-----------------------------------------------------------
Widget::~Widget()
{
    QSettings s;
    //QSettings sOut;
    s.setValue( "geometry", this->saveGeometry() );
    s.setValue( "pathIn",     pathFile_input     );
    s.setValue( "pathOut",     pathFile_output  );

    //получили старые значения с боксов
    Fold_save = ui->FdOld_SBX->value();
    Fnew_save = ui->FdNew_SBX->value();
    //сохранили их
    s.setValue("Fold_save", Fold_save);
    s.setValue("Fnew_save", Fnew_save);

    delete[] signal_buf;
    signal_buf = nullptr;
    delete ui;
}
//-----------------------------------------------------------

void Widget::on_input_PBN_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Выберите входной файл ", QFileInfo(pathFile_input).canonicalPath());

    if( !fileName.isEmpty()){
        log("Исходный файл '" + fileName + "' выбран");
        ui->input_LED->setText(QDir::toNativeSeparators(fileName));
        pathFile_input = fileName;
        flag_input = true; // чтобы разрешить нажать кнопку

    }else{
        log("Файл не выбран!");
    }
    //разрешаем нажать на start
    if(!ui->input_LED->text().isEmpty()   // проверяем еще на пустоту строк в LineEdit
       && !ui->output_LED->text().isEmpty()){
        ui->start_PBN->setEnabled(true);
    }
}
//-----------------------------------------------------------
void Widget::on_output_PBN_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Выберите файл ", QFileInfo(pathFile_output).canonicalPath());

    if( !fileName.isEmpty()){
        log("Путь выходного файла '" + fileName + "'");
        ui->output_LED->setText(QDir::toNativeSeparators(fileName));
        pathFile_output = fileName;
        flag_output = true; // чтобы разрешить нажать кнопку

    }else{
        log("Файл не выбран!");
    }
    //разрешаем нажать на start
    if( !ui->input_LED->text().isEmpty()
       && !ui->output_LED->text().isEmpty()){

        ui->start_PBN->setEnabled(true);
    }
}
//-----------------------------------------------------------
void Widget::on_start_PBN_clicked()
{
    ui->input_PBN->setEnabled(false); // пока обрабатываем не можем изменять кнопки
    ui->output_PBN->setEnabled(false);
    ui->FdNew_SBX->setEnabled(false);
    ui->FdNew_SL->setEnabled(false);
    ui->FdOld_SBX->setEnabled(false);
    ui->FdOld_SL->setEnabled(false);

    // значения частот дискретизации
    double FdOld = ui->FdOld_SBX->value();
    double FdNew = ui->FdNew_SBX->value();
    //пути до файлов
    QFile fileInput (ui->input_LED->text());
    QFile fileOutput (ui->output_LED->text());


    if( !fileInput.open(QIODevice::ReadOnly))
    {
        throw( "Ошибка при открытии исходного файла" );
        log("Ошибка при открытии исходного файла");
    }

    if( !fileOutput.open(QIODevice::WriteOnly) )
    {
        throw( "Ошибка при открытии файла с результатом"  );
        log("Ошибка при открытии файла с результатом");
    }
    log("Исходный файл '" + ui->input_LED->text() + "' выбран");
    log("Путь выходного файла '" + ui->output_LED->text() + "'");


    fileOutput.resize(0);
    fileInput.seek(0);
    ui->info_PBR->setValue(100*fileInput.pos() / fileInput.size());
    log("Обработка запущена");

    while(!fileInput.atEnd()){
        //че то делаем.....
        qApp->processEvents();
    }


    fileInput.close();
    fileOutput.close(); //
    ui->input_PBN->setEnabled(true); // после обработки можно выбрать файл повторно
    ui->output_PBN->setEnabled(true);
    ui->FdNew_SBX->setEnabled(false);
    ui->FdNew_SL->setEnabled(false);
    ui->FdOld_SBX->setEnabled(false);
    ui->FdOld_SL->setEnabled(false);
}
//-----------------------------------------------------------

//-----------------------------------------------------------
void Widget::log(const QString &s)
{
     QString m;
     m = QTime::currentTime().toString();
     m += "---";
     m += s;
     ui->log_PTE->appendPlainText( m );
     qDebug()<<m;
}
