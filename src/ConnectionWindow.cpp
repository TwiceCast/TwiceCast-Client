#include "ConnectionWindow.hpp"
#include "MainWindow.hpp"
#include "ui_connectionwindow.h"

ConnectionWindow::ConnectionWindow(NetworkManager *network, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ConnectionWindow),
    m_network(network)
{
    this->m_ui->setupUi(this);
    QFont font(this->m_ui->InfoConnectLabel->font());
    font.setPointSize(14);
    this->m_ui->InfoConnectLabel->setFont(font);
    this->move(QApplication::desktop()->screenGeometry(QApplication::desktop()->screenNumber(this)).center() - this->rect().center());
    this->setWindowFlags(this->windowFlags() & (~Qt::WindowContextHelpButtonHint));
}

ConnectionWindow::~ConnectionWindow(void)
{
    delete (m_ui);
}

void ConnectionWindow::tryReconnect(void)
{
    this->m_strike++;
    if (this->m_strike < 3)
        this->on_ConnectionButton_clicked();
    else
        this->failConnect("Cannot connect, check your connection");
}

void ConnectionWindow::failConnect(const QString &msg)
{
    this->m_ui->InfoConnectLabel->setStyleSheet("#InfoConnectLabel { color : red }");
    this->m_ui->InfoConnectLabel->setText("Error : " + msg);
}

void ConnectionWindow::on_ConnectionButton_clicked(void)
{
    QStringList list;

    list << "email=" + this->m_ui->UsernameEdit->text() << "password=" + this->m_ui->PasswordEdit->text();
    emit request(QNetworkAccessManager::PostOperation, "/user/login", list);
    this->m_ui->InfoConnectLabel->setStyleSheet("#InfoConnectLabel { color : blue }");
    this->m_ui->InfoConnectLabel->setText("Connecting...");
//    this->accept();
}

void ConnectionWindow::on_ExitButton_clicked(void)
{
    this->reject();
}
