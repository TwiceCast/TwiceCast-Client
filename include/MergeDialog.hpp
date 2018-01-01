#ifndef MERGEDIALOG_HPP
#define MERGEDIALOG_HPP

#include <QListWidget>
#include <QDialog>
#include <QMap>
#include <QFileInfo>

namespace Ui {
class MergeDialog;
}

class MergeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MergeDialog(QWidget *parent = 0);
    ~MergeDialog(void);

public:
    void init(const QString &);
    const QString &getID(void);
    const QMap<QString, QString> &getMergingContent(void);

public slots:
    void clean(void);
    void prepareFile(const QString &, const QString &);
    void compareChange(int);
    void stateChanged(QListWidgetItem *);

private:
    Ui::MergeDialog *m_ui;
    QString m_id;
    QMap<QString, QString> m_files;
};

#endif // MERGEDIALOG_HPP
