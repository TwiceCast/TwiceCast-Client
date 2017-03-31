#include "NetworkManager.hpp"

NetworkManager::NetworkManager(QObject *parent) :
    QObject(parent),
    m_network(new QNetworkAccessManager(this)),
    m_ws(new QWebSocket),
    m_timer(new QTimer),
    m_baseUrlApi("http://37.187.99.70"),
    m_baseUrlWs("ws://localhost:3005"),
    m_strike(0)
{
    connect(this->m_network, SIGNAL(finished(QNetworkReply*)), this, SIGNAL(responseReady(QNetworkReply*)));
    connect(this->m_ws, SIGNAL(textMessageReceived(QString)), this, SLOT(messageReceived(const QString &)));
    connect(this->m_ws, SIGNAL(pong(quint64,QByteArray)), this, SLOT(pongReceived()));
    connect(this->m_ws, SIGNAL(connected()), this, SIGNAL(wsConnected()));
    connect(this->m_timer, SIGNAL(timeout()), this, SLOT(pingSending()));
}

NetworkManager::~NetworkManager(void)
{
    delete (this->m_network);
    delete (this->m_timer);
    delete (this->m_ws);
}

QNetworkReply *NetworkManager::headRequest(const QString &url, const QStringList &) const
{
    QNetworkRequest request;

    request.setUrl(this->m_baseUrlApi + url);
    return (this->m_network->head(request));
}

QNetworkReply *NetworkManager::putRequest(const QString &url, const QStringList &parameters) const
{
    QNetworkRequest request;
    QString params;

    for (auto parameter : parameters)
        params += parameter + "\n";
    request.setUrl(this->m_baseUrlApi + url);
    return (this->m_network->put(request, params.toUtf8()));
}

QNetworkReply *NetworkManager::postRequest(const QString &url, const QStringList &parameters) const
{
    QNetworkRequest request;
    QString params;

    for (auto parameter : parameters)
        params += parameter + "\n";
    request.setUrl(this->m_baseUrlApi + url);
    return (this->m_network->post(request, params.toUtf8()));
}

QNetworkReply *NetworkManager::getRequest(const QString &url, const QStringList &parameters) const
{
    QNetworkRequest request;
    QString params;

    for (int i = 0; i < parameters.size(); i++)
        params += parameters[i] + (i == 0 ? "?" : "&");
    request.setUrl(this->m_baseUrlApi + url + params);
    return (this->m_network->get(request));
}

QNetworkReply *NetworkManager::deleteRequest(const QString &url, const QStringList &) const
{
    QNetworkRequest request;

    request.setUrl(this->m_baseUrlApi + url);
    return (this->m_network->deleteResource(request));
}

QNetworkReply *NetworkManager::customRequest(const QString &url, const QStringList &parameters) const
{
    QNetworkRequest request;

    if (parameters.length() < 1)
        return (NULL);
    request.setUrl(this->m_baseUrlApi + url);
#if QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 8
    QString params;

    for (int i = 1; i < parameters.length(); i++)
        params += parameters[i] + "\n";
    return (this->m_network->sendCustomRequest(request, parameters[0].toUtf8(), params.toUtf8()));
#else
    return (this->m_network->sendCustomRequest(request, parameters[0].toUtf8()));
#endif
}

QNetworkReply *NetworkManager::request(const QString &url, const QStringList &parameters, QNetworkAccessManager::Operation op) const
{
    static OpFunc tab[OP_NUMBER] = {
        {QNetworkAccessManager::HeadOperation, &NetworkManager::headRequest},
        {QNetworkAccessManager::PutOperation, &NetworkManager::putRequest},
        {QNetworkAccessManager::PostOperation, &NetworkManager::postRequest},
        {QNetworkAccessManager::GetOperation, &NetworkManager::getRequest},
        {QNetworkAccessManager::DeleteOperation, &NetworkManager::deleteRequest},
        {QNetworkAccessManager::CustomOperation, &NetworkManager::customRequest}
    };

    if (this->m_network->networkAccessible() == QNetworkAccessManager::NotAccessible)
        return (NULL);
    for (int i = 0; i < OP_NUMBER; i++) {
        if (tab[i].op == op)
            return ((this->*tab[i].func)(url, parameters));
    }
    return (NULL);
}

qint64 NetworkManager::sendBinaryMessage(const QByteArray &msg) const
{
    if (this->m_ws->isValid())
        return (this->m_ws->sendBinaryMessage(msg));
    return (0);
}

qint64 NetworkManager::sendTextMessage(const QString &msg) const
{
    if (this->m_ws->isValid())
        return (this->m_ws->sendTextMessage(msg));
    return (0);
}

bool NetworkManager::isConnected(void) const
{
    return (this->m_ws->isValid());
}

void NetworkManager::connectWs(void)
{
    QNetworkRequest request;

    request.setUrl(this->m_baseUrlWs);
    this->m_ws->open(request);
    this->m_timer->start(10000);
}

void NetworkManager::disconnectWs(void)
{
    this->m_timer->stop();
    this->m_ws->close();
}

void NetworkManager::pingSending(void)
{
    this->m_ws->ping();
    qDebug() << "Ping sent";
    this->m_strike++;
    if (this->m_strike >= 3) {
        //TODO : NETWORK ERROR
    }
}

void NetworkManager::pongReceived(void)
{
    qDebug() << "Pong received";
    this->m_strike = 0;
}

void NetworkManager::messageReceived(const QString &msg)
{
    qDebug() << msg;
}
