#include <QApplication>
#include "ConnectionWindow.hpp"
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ConnectionWindow connect;
    MainWindow *main;

    if (connect.exec() == QDialog::Rejected)
        return (0);
    main = new MainWindow(/*connect.retrieveData()*/);
    main->show();
    return (a.exec());
}
