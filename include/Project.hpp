#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

class Project
{
public:
    Project(const QString &, const QString &title = "Unknown");
    ~Project(void);

public:
    const QString &getPath(void) const;
    const QString &getTitle(void) const;
    const QString &getIgnoreFile(void) const;

public:
    void setTitle(const QString &);
    void setIgnoreFile(const QString &);

public:
    bool save(const QString &filename = ".tcconf");
    bool saveIgnored(QList<QString> &) const;

public:
    static Project *open(const QString &);
    QList<QString> retrieveIgnored(void) const;

private:
    QString m_path;
    QString m_title;
    QString m_ignoreFile;
};

#endif // PROJECT_HPP
