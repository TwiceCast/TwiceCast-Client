#ifndef CUSTOMPRITEM_H
#define CUSTOMPRITEM_H

#include <QApplication>
#include <QStyledItemDelegate>
#include <QString>
#include <QDateTime>
#include <QMouseEvent>
#include <QPainter>
#include <QTextDocument>

class CustomPRItem: public QStyledItemDelegate
{
    Q_OBJECT

public :
    explicit CustomPRItem(QObject *parent = NULL);
    ~CustomPRItem(void);

signals:
    void mergeRequest(const QModelIndex &);
    void mergeDelete(const QModelIndex &);

public:
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *, QAbstractItemModel *, const QStyleOptionViewItem &, const QModelIndex &);
};

#endif // CUSTOMPRITEM_H
