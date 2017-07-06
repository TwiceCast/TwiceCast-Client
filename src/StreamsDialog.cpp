#include "StreamsDialog.hpp"
#include "ui_StreamsDialog.h"

StreamsDialog::StreamsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::StreamsDialog)
{
    this->m_ui->setupUi(this);
}

StreamsDialog::~StreamsDialog()
{
    delete this->m_ui;
}

void StreamsDialog::setStreamList(const QList<Stream *> &list)
{
    this->m_list = list;
    this->m_ui->comboBox->clear();
    for (auto stream : this->m_list)
        this->m_ui->comboBox->addItem(stream->getName());
}

Stream *StreamsDialog::getSelected(void) const
{
    return (this->m_list[this->m_ui->comboBox->currentIndex()]);
}
