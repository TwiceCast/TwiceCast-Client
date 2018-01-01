#ifndef CUSTOMSTYLE_HPP
#define CUSTOMSTYLE_HPP

#include <QApplication>
#include <QStyledItemDelegate>

#define COLUMN_NAME 0
#define COLUMN_PROGRESS 1
#define COLUMN_CHECKSTATE 2
#define COLUMN_PATH 3

class CustomStyle : public QStyledItemDelegate
{
    Q_OBJECT

public :
    explicit CustomStyle(QObject *parent = NULL);
    ~CustomStyle(void);

public:
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
};

#endif // CUSTOMSTYLE_HPP
