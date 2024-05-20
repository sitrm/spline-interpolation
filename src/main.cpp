#include "widget.h"
#include <QApplication>
//Изменение частоты дискретизации комплексного 32-х разрядного сигнала float методом сплайн интерполяции

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("spline-2024");
    a.setOrganizationName("LegionFurie");

    Widget w;
    w.show();
    return a.exec();
}
