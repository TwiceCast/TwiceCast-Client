#include "ProjectOpenDialog.hpp"
#include "ui_projectopendialog.h"
#include <QDebug>

ProjectOpenDialog::ProjectOpenDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ProjectOpenDialog)
{
    this->m_ui->setupUi(this);
    this->m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    for (int i = 0; i < 5; i++) {
        this->m_oldProject[i] = new QLabel;
        this->m_oldProject[i]->setTextInteractionFlags(Qt::TextBrowserInteraction);
        connect(this->m_oldProject[i], SIGNAL(linkActivated(QString)), this, SLOT(oldProjectOpen(QString)));
    }
}

ProjectOpenDialog::~ProjectOpenDialog(void)
{
    for (int i = 0; i < 5; i++)
        if (this->m_oldProject[i] != NULL)
            delete (this->m_oldProject[i]);
    delete (this->m_ui);
}

void ProjectOpenDialog::init(User *user)
{
    QFile conf(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/" + user->getUsername() + "/.oldPath"));
    QTextStream stream;
    Project *project;
    QStringList olds;
    QString line;
    int number = 0;

    if (!conf.open(QFile::ReadOnly))
        return;
    stream.setDevice(&conf);
    while (!stream.atEnd()) {
        line = stream.readLine();
        if (line.trimmed() != "")
            olds.append(line);
    }
    for (int i = 0; i < olds.size() && number < 5; i++)
        if ((project = Project::open(olds[i])) != NULL) {
            this->m_oldProject[number]->setText("<a href=\"" + project->getPath() + "\">" + project->getTitle() + "</a>");
            this->m_ui->oldPathWidget->layout()->addWidget(this->m_oldProject[number]);
            number++;
        }
    conf.close();
}

Project *ProjectOpenDialog::getProject(void)
{
    return (Project::open(this->m_ui->pathLine->text() + "/.tcconf"));
}

void ProjectOpenDialog::oldProjectOpen(const QString &path)
{
    this->m_ui->pathLine->setText(path);
    this->accept();
}

void ProjectOpenDialog::on_pathButton_clicked(void)
{
    QString path = QFileDialog::getExistingDirectory(this, "Open project",
                                                     "", QFileDialog::ShowDirsOnly);

    this->m_ui->pathLine->setText(path);
}

void ProjectOpenDialog::on_pathLine_textChanged(const QString &)
{
    this->m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(this->m_ui->openTab->currentIndex() == 0 &&
                                                                    this->m_ui->pathLine->text() != "");
}
