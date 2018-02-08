#ifndef MERGEDIALOG_HPP
#define MERGEDIALOG_HPP

#include <QListWidget>
#include <QDialog>
#include <QMap>
#include <QFileInfo>
#include <QDir>

#include "dtl/dtl.hpp"

namespace Ui {
class MergeDialog;
}

class MergeDialog : public QDialog
{
    Q_OBJECT

    struct LineCheck {
      QString line;
      bool checked;
    };

public:
    explicit MergeDialog(QWidget *parent = 0);
    ~MergeDialog(void);

private:
    QVector<std::string> getBasicVector(const QStringList &);
    void showFileLines(const QVector<LineCheck> &);

public:
    void init(const QString &);
    void setProjectPath(const QString &);
    const QString &getID(void);
    const QMap<QString, QStringList> getMergingContent(void);

public slots:
    void clean(void);
    void prepareFile(const QString &, const QString &);
    void compareChange(int);
    void stateChanged(QListWidgetItem *);
    void lineChanged(QListWidgetItem *);

private:
    Ui::MergeDialog *m_ui;
    QString m_id;
    QString m_basePath;
    QMap<QString, QVector<LineCheck>> m_files;
    int m_selected;
};

#endif // MERGEDIALOG_HPP
