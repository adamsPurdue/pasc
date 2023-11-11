#include "pasc.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    pasc w;
    w.show();
    return a.exec();
}
