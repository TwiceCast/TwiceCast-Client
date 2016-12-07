#include "ProjectCreationDialog.hpp"
#include "ui_projectcreationdialog.h"

ProjectCreationDialog::ProjectCreationDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ProjectCreationDialog)
{
    m_ui->setupUi(this);
}

ProjectCreationDialog::~ProjectCreationDialog(void)
{
    delete (m_ui);
}

void ProjectCreationDialog::on_browseButton_clicked(void)
{
    QString path = QFileDialog::getExistingDirectory(this,
                                                     "Project path",
                                                     "");

    if (path == "")
        return;
    this->m_ui->linePath->setText(path);
}

QMap<ProjectCreationDialog::ProjectSettings, QString> ProjectCreationDialog::getValues(void) const
{
    QMap<ProjectSettings, QString> result;

    result[TITLE] = this->m_ui->lineTitle->text();
    result[PATH] = this->m_ui->linePath->text();
    return (result);
}
