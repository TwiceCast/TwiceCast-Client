#include "NetworkManager.hpp"

NetworkManager::NetworkManager(QObject *parent) :
    QObject(parent),
    m_network(new QNetworkAccessManager(this)),
    m_baseUrl("")
{
    connect(this->m_network, SIGNAL(finished(QNetworkReply*)), this, SIGNAL(responseReady(QNetworkReply*)));
}

NetworkManager::~NetworkManager(void)
{
    delete (this->m_network);
}

QNetworkReply *NetworkManager::request(const QString &url, const QStringList &parameters, QNetworkAccessManager::Operation op)
{
    QNetworkReply *reply;
    QNetworkRequest request;
    QString paramsWritten, paramsUrl;

    if (this->m_network->networkAccessible() == QNetworkAccessManager::NotAccessible)
        return (NULL);
    request.setUrl(QUrl(this->m_baseUrl + url));
    for (auto param : parameters)
        paramsWritten += param + "\n";
    for (int i = 0; i < parameters.size(); i++)
        paramsUrl += (i == 0 ? "" : "&") + parameters[i];
    switch (op) {
    case QNetworkAccessManager::HeadOperation :
        reply = this->m_network->head(request);
        break;
    case QNetworkAccessManager::PutOperation :
        reply = this->m_network->put(request, paramsWritten.toUtf8());
        break;
    case QNetworkAccessManager::PostOperation :
        reply = this->m_network->post(request, paramsWritten.toUtf8());
        break;
    case QNetworkAccessManager::GetOperation :
        request.setUrl(QUrl(this->m_baseUrl + url + "?" + paramsUrl));
        reply = this->m_network->get(request);
        break;
    case QNetworkAccessManager::DeleteOperation :
        reply = this->m_network->deleteResource(request);
        break;
    case QNetworkAccessManager::CustomOperation :
        break;
    default:
        reply = NULL;
    }
    return (reply);
}
