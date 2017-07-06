#ifndef STREAM_HPP
#define STREAM_HPP

#include <QString>

class Stream
{
public:
    Stream(void);
    ~Stream(void);

public:
    QString getName(void) const;
    QString getWsToken(void) const;
    QString getId(void) const;
    QString getWsUrl(void) const;

public:
    void setName(const QString &);
    void setWsToken(const QString &);
    void setId(const QString &);
    void setWsUrl(const QString &);

private:
    QString m_name;
    QString m_wsUrl;
    QString m_wsToken;
    QString m_id;
};

#endif // STREAM_HPP
