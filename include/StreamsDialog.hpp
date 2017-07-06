#ifndef STREAMSDIALOG_HPP
#define STREAMSDIALOG_HPP

#include <QDialog>

#include "Stream.hpp"

namespace Ui {
class StreamsDialog;
}

class StreamsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StreamsDialog(QWidget *parent = 0);
    ~StreamsDialog();

public:
    void setStreamList(const QList<Stream *> &);

public:
    Stream *getSelected(void) const;

private:
    Ui::StreamsDialog *m_ui;
    QList<Stream *> m_list;
};

#endif // STREAMSDIALOG_HPP
