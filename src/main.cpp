#include <QApplication>
#include "ConnectionWindow.hpp"
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow win;

    Q_UNUSED(win);
    return (a.exec());
}
