#include "mainwindow.h"
#include "memory.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.setWindowTitle("OS Memory Management Simulator");
    w.show();

    memory* mem = new memory(&w);
    app.connect(&app, &QApplication::lastWindowClosed, mem, &memory::stop);//关闭窗口后保证循环结束

    int x = app.exec();
    delete mem;
    return x;
}
