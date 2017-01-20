#include "ConnectionWindow.hpp"
#include "MainWindow.hpp"
#include "ui_connectionwindow.h"

ConnectionWindow::ConnectionWindow(NetworkManager *network, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ConnectionWindow),
    m_network(network)
{
    this->m_ui->setupUi(this);
    this->move(QApplication::desktop()->screenGeometry(QApplication::desktop()->screenNumber(this)).center() - this->rect().center());
    this->setWindowFlags(this->windowFlags() & (~Qt::WindowContextHelpButtonHint));
}

ConnectionWindow::~ConnectionWindow(void)
{
    delete (m_ui);
}

void ConnectionWindow::on_ConnectionButton_clicked(void)
{
//    QStringList list;

//    list << "username=" + this->m_ui->UsernameEdit->text() << "password=" + this->m_ui->PasswordEdit->text();
//    this->m_network->request("users", list, QNetworkAccessManager::GetOperation);
    this->accept();
}

void ConnectionWindow::on_ExitButton_clicked(void)
{
    this->reject();
}
