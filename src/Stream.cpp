#include "Stream.hpp"

Stream::Stream(void)
{

}

Stream::~Stream(void)
{

}

QString Stream::getName(void) const
{
    return this->m_name;
}

QString Stream::getWsToken(void) const
{
    return this->m_wsToken;
}

QString Stream::getId(void) const
{
    return this->m_id;
}

QString Stream::getWsUrl(void) const
{
    return this->m_wsUrl;
}

void Stream::setName(const QString &name)
{
    this->m_name = name;
}

void Stream::setWsToken(const QString &wsToken)
{
    this->m_wsToken = wsToken;
}

void Stream::setId(const QString &id)
{
    this->m_id = id;
}

void Stream::setWsUrl(const QString &wsUrl)
{
    this->m_wsUrl = wsUrl;
}
