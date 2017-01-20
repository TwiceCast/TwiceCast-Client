#include <QDebug>
#include "Project.hpp"

Project::Project(const QString &path, const QString &title) :
    m_path(path),
    m_title(title),
    m_ignoreFile(".tcignore")
{
    if (this->m_title == "")
        this->m_title = "Unknown";
}

Project::~Project(void)
{

}

const QString &Project::getPath(void) const
{
    return (this->m_path);
}

const QString &Project::getTitle(void) const
{
    return (this->m_title);
}

const QString &Project::getIgnoreFile(void) const
{
    return (this->m_ignoreFile);
}

void Project::setTitle(const QString &title)
{
    this->m_title = title;
}

void Project::setIgnoreFile(const QString &ignoreFile)
{
    this->m_ignoreFile = ignoreFile;
}

bool Project::saveIgnored(QList<QString> &list) const
{
    QFile file(this->m_path + "/" + this->m_ignoreFile);
    QList<QString> before, after;
    QTextStream stream(&file);
    int tcindex;

    if (!file.open(QFile::ReadWrite | QFile::Text))
        return (false);
    while (!stream.atEnd())
        before.append(stream.readLine());
    for (auto line : before)
        if (line.trimmed()[0] == '#' || line.trimmed() == "")
            after.append(line);
        else
            for (int i = 0; i < list.size(); i++)
                if (list[i] == line) {
                    after.append(line);
                    list.erase(list.begin() + i);
                    break;
                }
    if (!list.isEmpty()) {
        for (tcindex = 0; tcindex < after.size() && after[tcindex] != "# Added by TwiceCast Client"; tcindex++);
        if (tcindex == after.size()) {
            if (tcindex == 0 || after[tcindex - 1].trimmed() != "") {
                after.append("");
                tcindex++;
            }
            after.append("# Added by TwiceCast Client");
            after.append("");
        }
        tcindex += 2;
        for (auto ignore : list)
            after.insert(tcindex, ignore);
    }
    file.resize(0);
    for (auto line : after)
        stream << line << "\n";
    file.close();
    return (true);
}

QList<QString> Project::retrieveIgnored(void) const
{
    QFile file(this->m_path + "/" + this->m_ignoreFile);
    QTextStream stream(&file);
    QList<QString> result;
    QString line;

    if (!file.exists())
        return (result);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return (result);
    while (!stream.atEnd()) {
        line = stream.readLine();
        if (line.trimmed()[0] != '#' && line.trimmed() != "")
            result.append(line);
    }
    return (result);
}

bool Project::save(const QString &filename)
{
    QFile file(this->m_path + "/" + filename), check;
    QXmlStreamWriter xml(&file);

    if (file.exists())
        return (false);
    if (!file.open(QFile::WriteOnly))
        return (false);
    if (check.exists(this->m_path + "/.gitignore"))
        if (QMessageBox::information(NULL, "Ignore file", "There is a .gitignore in the project, do you want to use it as ignore file?",
                                     QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            this->m_ignoreFile = ".gitignore";
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("config");
    xml.writeTextElement("title", this->m_title);
    xml.writeEmptyElement("ignore");
    xml.writeAttribute("src", this->m_ignoreFile);
    xml.writeEndElement();
    xml.writeEndDocument();
    file.close();
    return (true);
}

Project *Project::open(const QString &path)
{
    QFile file(path + "/.tcconf");
    Project *project = new Project(path);
    QXmlStreamReader xml(&file);

    if (!file.exists())
        return (NULL);
    if (!file.open(QFile::ReadOnly))
        return (NULL);
    while (!xml.atEnd()) {
        if (xml.isStartElement() && xml.name() == "title")
            project->setTitle(xml.readElementText());
        if (xml.isStartElement() && xml.name() == "ignore")
            project->setIgnoreFile(xml.attributes().value("src").toString());
        xml.readNext();
    }
    if (xml.hasError())
        qDebug() << xml.errorString();
    file.close();
    return (project);
}
