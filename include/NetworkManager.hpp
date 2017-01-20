#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include <QObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = 0);
    ~NetworkManager(void);

public:
    QNetworkReply *request(const QString &, const QStringList &, QNetworkAccessManager::Operation);

signals:
    void responseReady(QNetworkReply *);

public slots:

private:
    QNetworkAccessManager *m_network;
    QString m_baseUrl;
};

#endif // NETWORKMANAGER_HPP
