#include <QApplication>
#include "ConnectionWindow.hpp"
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow main;

    if (main.connectUser() == QDialog::Rejected)
        return (0);
    main.show();
    return (a.exec());
}
