#include "ProjectOpenDialog.hpp"
#include "ui_projectopendialog.h"

ProjectOpenDialog::ProjectOpenDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ProjectOpenDialog)
{
    this->m_ui->setupUi(this);
    this->m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

ProjectOpenDialog::~ProjectOpenDialog(void)
{
    delete (this->m_ui);
}

Project *ProjectOpenDialog::getProject(void)
{
    return (Project::open(this->m_ui->pathLine->text()));
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
