#ifndef CONNECTIONWINDOW_HPP
#define CONNECTIONWINDOW_HPP

#include <QDialog>

namespace Ui {
class ConnectionWindow;
}

class ConnectionWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionWindow(QWidget *parent = 0);
    ~ConnectionWindow();

private slots:
    void on_ConnectionButton_clicked();
    void on_ExitButton_clicked();

private:
    Ui::ConnectionWindow *m_ui;
};

#endif // CONNECTIONWINDOW_HPP
