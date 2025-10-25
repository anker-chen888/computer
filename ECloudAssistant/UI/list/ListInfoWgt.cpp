#include "ListInfoWgt.h"
#include <QVBoxLayout>      // 布局管理器
#include "ListWidget.h"
#include "CustomWgt.h"
#include "StyleLoader.h"    // 加载样式表

// 包含一个按钮(登录)和一个列表部件。列表中包含三个自定义部件，每个部件都有图片和文本
// 点击列表中的项目会发射信号
ListInfoWgt::ListInfoWgt(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setFixedSize(200,540);

    userBtn_ = new QPushButton(this);
    listWgt_ = new ListWidget(this);

    userBtn_->setObjectName("user_Btn");
    listWgt_->setObjectName("listWidget");

    connect(listWgt_,&ListWidget::itemCliked,this,&ListInfoWgt::HandleItemSelect);

    // 点击用户图标进行登录，发射sig_Select(0)信号，并取消所有自定义部件的高亮显示，同时清除列表的选择
    connect(userBtn_,&QPushButton::clicked,this,[this](){
        emit sig_Select(0);
        for(auto item : customWgts_)
        {
            //取消高亮
            item->setHightLight(false);
        }
        listWgt_->clearSelection();
    });

    //添加item
    CustomWgt* rmtWgt = new CustomWgt(this);    // 远程列表
    CustomWgt* dvcWgt = new CustomWgt(this);    // 设备列表
    CustomWgt* setWgt = new CustomWgt(this);    // 高级设置

    //更新他们的图片
    rmtWgt->setImageAndText("远程控制",":/UI/brown/list/remote.png",":/UI/brown/list/remote_press.png",true);
    dvcWgt->setImageAndText("设备列表",":/UI/brown/list/device.png",":/UI/brown/list/device_press.png");
    setWgt->setImageAndText("高级设置",":/UI/brown/list/setting.png",":/UI/brown/list/setting_press.png");

    //将这些customwgt存起来
    customWgts_.push_back(rmtWgt);
    customWgts_.push_back(dvcWgt);
    customWgts_.push_back(setWgt);

    listWgt_->AddWidget(rmtWgt);
    listWgt_->AddWidget(dvcWgt);
    listWgt_->AddWidget(setWgt);


    userBtn_->setFixedSize(100,100);
    listWgt_->setFixedSize(200,340);

    //布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addSpacing(50);
    layout->addWidget(userBtn_,0,Qt::AlignHCenter);
    layout->addSpacing(50);
    layout->addWidget(listWgt_,1,Qt::AlignHCenter);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    StyleLoader::getInstance()->loadStyle(":/UI/brown/main.css",this);
}

void ListInfoWgt::HandleItemSelect(int index) //0
{
    if(index < 0 || (index > customWgts_.size() - 1))
    {
        return;
    }
    emit sig_Select(index + 1);
    for(int i = 0; i < customWgts_.size();i++)
    {
        if(i == index)
        {
            //设置高亮
            customWgts_[i]->setHightLight(true);
        }
        else
        {
            customWgts_[i]->setHightLight(false);
        }
    }
}
