#ifndef CONNECTIONWINDOW_HPP
#define CONNECTIONWINDOW_HPP

#include <QDialog>
#include <QDesktopWidget>

#include "NetworkManager.hpp"

namespace Ui {
class ConnectionWindow;
}

class ConnectionWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionWindow(NetworkManager *, QWidget *parent = 0);
    ~ConnectionWindow();

public:
    void tryReconnect(void);
    void failConnect(const QString &);

signals:
    void request(QNetworkAccessManager::Operation, const QString &, const QStringList &, const QStringList &headers = QStringList());

private slots:
    void on_ConnectionButton_clicked();
    void on_ExitButton_clicked();

private:
    Ui::ConnectionWindow *m_ui;
    NetworkManager *m_network;
    int m_strike;
};

#endif // CONNECTIONWINDOW_HPP
