#include "MainWgt.h"
#include "RemoteWgt.h"
#include "LoginWgt.h"   

// 管理多个字窗口，并通过堆叠窗口小部件来实现页面的切换
MainWgt::MainWgt(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_StyledBackground);
    setFixedSize(600,510);

    stackWgt_ = new QStackedWidget(this);   // 管理多个子页面
    stackWgt_->setFixedSize(600,510);
    // 创建四个子页面
    login_ = new LoginWgt(this);     // 登录页面
    remoteWgt_= new RemoteWgt(this); // 远程控制页面
    deviceWgt_= new QWidget(this);   // 设备列表页面--暂无
    settingWgt_= new QWidget(this);  // 设置页面------暂无

    deviceWgt_->setFixedSize(600,510);
    settingWgt_->setFixedSize(600,510);

    deviceWgt_->setStyleSheet("background-color: #664764");
    settingWgt_->setStyleSheet("background-color: #957522");

    stackWgt_->addWidget(login_); //0
    stackWgt_->addWidget(remoteWgt_); //1
    stackWgt_->addWidget(deviceWgt_); //2
    stackWgt_->addWidget(settingWgt_);//3

    // 将login_按钮与登录页面绑定
    connect(login_,&LoginWgt::sig_logined,remoteWgt_,&RemoteWgt::handleLogined);

    //指定显示哪个窗口
    stackWgt_->setCurrentWidget(remoteWgt_);
}

// 根据槽函数实现页面切换
void MainWgt::slot_ItemCliked(int index)
{
    QWidget* widget = stackWgt_->widget(index);
    if(widget)
    {
        stackWgt_->setCurrentWidget(widget);
    }
}
