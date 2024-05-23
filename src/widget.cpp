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
#include <QDir>
//-----------------------------------------------------------
#include "ippCustom.h"
#include "spline.h"
#include "ipps.h"
//-----------------------------------------------------------
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    signal_buf = new Ipp32fc [BUF_SIZE]; // немедленно
    time_buf = new double [BUF_SIZE];

    QSettings s;

    this->restoreGeometry(s.value("geometry").toByteArray());
    //сохранили пути на кнопках
    pathFile_input = s.value( "pathIn", QDir::homePath() ).toString();
    pathFile_output = s.value( "pathOut", QDir::homePath() ).toString();

    ui->input_LED->setText(QDir::toNativeSeparators(pathFile_input));
    ui->output_LED->setText(QDir::toNativeSeparators(pathFile_output));

    ui->FdNew_SBX->setValue(s.value("Fnew_save", 0).toInt());
    ui->FdOld_SBX->setValue(s.value("Fold_save", 0).toInt());

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
    s.setValue( "geometry", this->saveGeometry() );
    s.setValue( "pathIn",     pathFile_input     );
    s.setValue( "pathOut",     pathFile_output   );


    s.setValue("Fold_save", ui->FdOld_SBX->value());
    s.setValue("Fnew_save", ui->FdNew_SBX->value());

    delete[] signal_buf;      signal_buf = nullptr;
    delete[] time_buf;        time_buf = nullptr;
    delete ui;
}
//-----------------------------------------------------------

void Widget::on_input_PBN_clicked()
{
    QString s;
    s = QFileDialog::getOpenFileName(this,"Выберите входной файл ",
                                                    QFileInfo(pathFile_input).canonicalPath(),
                                                    "Все (*.*);;PCM(*.pcm)");

    if( !s.isEmpty()){
        log("Исходный файл '" + s + "' выбран");
        ui->input_LED->setText(QDir::toNativeSeparators(s));
        pathFile_input = s;


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
    QString s;
    s = QFileDialog::getOpenFileName(this,"Выберите выходной файл ",
                                                    pathFile_output,
                                                    "Все (*.*);;PCM(*.pcm)");
//QFileInfo(pathFile_output).canonicalPath()
    if( !s.isEmpty()){
        log("Путь выходного файла '" + s + "'");
        ui->output_LED->setText(QDir::toNativeSeparators(s));
        pathFile_output = s;


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
    double SRold = ui->FdOld_SBX->value();
    double SRnew = ui->FdNew_SBX->value();
    //пути до файлов
    QFile fileInput (pathFile_input);
    QFile fileOutput (pathFile_output);


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


    fileOutput.resize(0);
    fileInput.seek(0);//позиция в файле на начало

    ui->info_PBR->setValue(100*fileInput.pos() / fileInput.size()); // progressBar

    log("Обработка запущена...");
    //-------------
    int size_block;             // размер блока
    int reed_block_count = 0;  //колво считанный блоков
    //temp
    int count = 0;
    int ccount = 0;
    int n = 0;
    int m = 0;
    Ipp32fc new_signal[1];


    while(!fileInput.atEnd()){

        size_block = fileInput.read((char*)signal_buf, sizeof(Ipp32fc)*BUF_SIZE);
        size_block /= sizeof(Ipp32fc);
        for(int i = 0; i < size_block; i++){
            time_buf[i] = (i+(BUF_SIZE)*reed_block_count)/ SRold; // временные отметки для сигнала
        }

        std::vector<double> X;  // время
        std::vector<double> RE; //деййствительная часть сигнала
        std::vector<double> IM; //мнимая часть сигнала

        for(int i = 0; i < size_block; i++){
            X.push_back(time_buf [i]);
            RE.push_back(signal_buf[i].re);
            IM.push_back(signal_buf[i].im);
        }

        tk::spline s_re(X,RE);
        tk::spline s_im(X,IM);

        if(fileInput.pos() != fileInput.size()) // проверяем достигнут ли конец
        {
            fileInput.seek(fileInput.pos()-sizeof(Ipp32fc)); // сдвигаем позицию в файле и сдвигаем на один элемент
                                                        //чтобы избежать повторного чтения последних данных в следующем блоке
        }
        reed_block_count++;  // плюс считанный блок
       // qDebug() << reed_block_count;

        while ((n+ccount)/SRnew < time_buf[size_block-1])   // цикл интерполяции
        {
            if((n+ccount)/SRnew < time_buf[m])
            {
                new_signal[0].re = s_re((n+ccount)/SRnew);
                new_signal[0].im = s_im((n+ccount)/SRnew);
                fileOutput.write((char*)new_signal,sizeof(Ipp32fc));
            }
            else
            {
                m++;
                new_signal[0].re = s_re((n+ccount)/SRnew);
                new_signal[0].im = s_im((n+ccount)/SRnew);
                fileOutput.write((char*)new_signal,sizeof(Ipp32fc));
            }

            // Увеличиваются индексы для сглаживания и временного отсчета
            count++;
            n++;
        }
        //обновляем для след цикла
        ccount = count;
        n = 0; m = 0;
        ui->info_PBR->setValue(100*fileInput.pos()/fileInput.size());

        X.clear();
        RE.clear();
        IM.clear();
    }


    fileInput.close();
    fileOutput.close();
    // после обработки разрешаем нажимать на кнопки
    ui->input_PBN->setEnabled(true);
    ui->output_PBN->setEnabled(true);
    ui->FdNew_SBX->setEnabled(true);
    ui->FdNew_SL->setEnabled(true);
    ui->FdOld_SBX->setEnabled(true);
    ui->FdOld_SL->setEnabled(true);
    log("Обработка завершена успешно");
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
