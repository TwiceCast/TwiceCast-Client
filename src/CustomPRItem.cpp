#include "CustomPRItem.hpp"
#include <QDebug>

CustomPRItem::CustomPRItem(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

CustomPRItem::~CustomPRItem(void)
{

}

void CustomPRItem::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect;
    QFont defaultFont = painter->font();
    QString title = index.data(Qt::DisplayRole).toString();
    QString author = index.data(Qt::UserRole + 1).toString();
    QString description = index.data(Qt::UserRole + 2).toString();
    QDateTime date = index.data(Qt::UserRole + 3).toDateTime();
    QStyleOptionButton button;

    rect = option.rect.adjusted(0, 5, 0, 0);
    painter->setFont(QFont(defaultFont.family(), 14, -1, true));
    painter->drawText(rect, title, Qt::AlignTop | Qt::AlignHCenter);
    rect = option.rect.adjusted(5, 35, 0, 0);
    painter->setFont(defaultFont);
    painter->drawText(rect, "Author : " + author + "\nDescription : " + description + "\nDate : " + date.toString("hh:mm dd/MM/yy"), Qt::AlignTop | Qt::AlignLeft);
    rect = option.rect.adjusted(0, option.rect.height() - 30, -(option.rect.width() / 2), 0);
    button.rect = rect;
    button.text = "Merge";
    button.state = QStyle::State_Enabled | (index.data(Qt::UserRole + 5).toBool() ? QStyle::State_Sunken : QStyle::State_Raised);
    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
    rect = option.rect.adjusted(option.rect.width() / 2, option.rect.height() - 30, 0, 0);
    button.rect = rect;
    button.text = "Delete";
    button.state = QStyle::State_Enabled | (index.data(Qt::UserRole + 6).toBool() ? QStyle::State_Sunken : QStyle::State_Raised);
    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
    painter->drawLine(0, option.rect.height(), option.rect.width(), option.rect.height());
}

QSize CustomPRItem::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const
{
    return QSize(option.rect.width(), 160);
}

bool CustomPRItem::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *e = (QMouseEvent *)event;
        QRect rect = option.rect.adjusted(0, option.rect.height() - 30, -(option.rect.width() / 2), 0);

        if (e->button() != Qt::LeftButton)
            return false;
        model->setData(index, false, Qt::UserRole + 5);
        model->setData(index, false, Qt::UserRole + 6);
        if (rect.contains(e->pos())) {
            qDebug() << "Merge request";
            emit mergeRequest(index);
        }
        rect = option.rect.adjusted(option.rect.width() / 2, option.rect.height() - 30, 0, 0);
        if (rect.contains(e->pos())) {
            qDebug() << "Merge delete";
            emit mergeDelete(index);
        }
        return true;
    }
    else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = (QMouseEvent *)event;
        QRect rect = option.rect.adjusted(0, option.rect.height() - 30, -(option.rect.width() / 2), 0);

        if (e->button() != Qt::LeftButton)
            return false;
        if (rect.contains(e->pos()))
            model->setData(index, true, Qt::UserRole + 5);
        rect = option.rect.adjusted(option.rect.width() / 2, option.rect.height() - 30, 0, 0);
        if (rect.contains(e->pos()))
            model->setData(index, true, Qt::UserRole + 6);
        return true;
    }
    return false;
}
