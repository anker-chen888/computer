#include "ListWidget.h"
#include "CustomWgt.h"
#include <QMouseEvent>

// 提供一个AddWidget，用于将自定义的Custom部件添加到列表中
// 并重写鼠标按下事件处理函数，当用户点击列表中的某个项时，发送一个信号ItemCliked
ListWidget::ListWidget(QWidget *parent)
    :QListWidget(parent)
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setCurrentRow(0);
}

ListWidget::~ListWidget()
{

}

void ListWidget::AddWidget(CustomWgt *widget)
{
    QListWidgetItem* listWidgetItem = new QListWidgetItem(this);    // 创建一个新的列表项
    listWidgetItem->setSizeHint(widget->sizeHint());
    this->setItemWidget(listWidgetItem, widget);
}

// 鼠标按下事件
void ListWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        QListWidgetItem* item = itemAt(event->pos());
        if(item)
        {
            int index = row(item);
            //发送信号
            emit itemCliked(index);
        }
        QListWidget::mousePressEvent(event);
    }
}
