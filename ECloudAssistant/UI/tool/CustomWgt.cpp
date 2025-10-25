﻿#include "CustomWgt.h"
#include <QHBoxLayout>
#include <QFont>

CustomWgt::CustomWgt(QWidget *parent)
    : QWidget{parent}
    ,imagePath_(2,"")
{
    setWindowFlag(Qt::FramelessWindowHint);
    setFixedSize(180,30);
    Init();
}

CustomWgt::~CustomWgt()
{
    imagePath_.clear();
}

// 设置高亮状态
void CustomWgt::setHightLight(bool flag)
{
    const QString str = flag ? imagePath_[1] : imagePath_[0];
    setPicture(str);
}

// 设置图片和文本
void CustomWgt::setImageAndText(const QString &text, const QString &normal, const QString &hightlight, bool ishigtlight)
{
    if(normal.isEmpty() || hightlight.isEmpty())
    {
        return;
    }
    imagePath_[0] = normal;
    imagePath_[1] = hightlight;
    const QString str = ishigtlight ? hightlight : normal;
    setPicture(str);
    setLableTxt(text);
}

// 创建两个QLabel子部件，分别用于显示图片和文本
void CustomWgt::Init()
{
    name_ = new QLabel(this);
    image_ = new QLabel(this);
    image_->setFixedSize(20,20);
    name_->setFixedSize(100,20);

    image_->setStyleSheet("background-color:transparent;border:none;");
    name_->setStyleSheet("background-color:transparent;border:none;");

    QHBoxLayout* layout = new QHBoxLayout(this);

    layout->addWidget(image_);
    layout->addWidget(name_);
    setLayout(layout);
}

// 设置文本
void CustomWgt::setLableTxt(const QString &text)
{
    name_->setText(text);
    QFont font("Microsoft Yahei",11);
    name_->setFont(font);
    name_->setStyleSheet("color:#FFFFFF");
}

// 设置图片
void CustomWgt::setPicture(const QString &imagepath)
{
    QPixmap pixmap;
    if(pixmap.load(imagepath))
    {
        image_->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        image_->setPixmap(pixmap.scaled(image_->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    }
}
