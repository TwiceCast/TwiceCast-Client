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

Project *ProjectCreationDialog::getProject(void) const
{
    return (new Project(this->m_ui->linePath->text(), this->m_ui->lineTitle->text()));
}

void ProjectCreationDialog::on_linePath_textChanged(const QString &)
{
    this->m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(this->m_ui->linePath->text() != "");
}
