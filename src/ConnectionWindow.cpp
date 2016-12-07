#include "ConnectionWindow.hpp"
#include "MainWindow.hpp"
#include "ui_connectionwindow.h"

ConnectionWindow::ConnectionWindow(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ConnectionWindow)
{
    this->m_ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & (~Qt::WindowContextHelpButtonHint));
}

ConnectionWindow::~ConnectionWindow()
{
    delete (m_ui);
}

void ConnectionWindow::on_ConnectionButton_clicked()
{
//    if (!this->userExists())
//        return;
    this->accept();
}

void ConnectionWindow::on_ExitButton_clicked()
{
    this->reject();
}
