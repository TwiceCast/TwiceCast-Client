#include "CustomStyle.hpp"

CustomStyle::CustomStyle(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

CustomStyle::~CustomStyle(void)
{

}

void CustomStyle::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == COLUMN_PROGRESS) {
        int progress = index.data().toInt();

        if (progress == -2)
            return;
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = option.rect;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = (progress == -1 ? 0 : progress);
        progressBarOption.text = (progress == -1 ? "Not sync" : progress == 100 ? "Done" : QString::number(progress) + "%");
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = true;

        QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                           &progressBarOption, painter);
    }
    else
        QStyledItemDelegate::paint(painter, option, index);
}
