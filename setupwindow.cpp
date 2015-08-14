#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
//using layout to write form by my self
SetupWindow::SetupWindow()
{
    /*设置图标和标题*/
    setWindowTitle("设置");
    QIcon prismicon=QIcon(":/windowlogo/logo");
    setWindowIcon(prismicon);
    /*注册表
    http://www.cnblogs.com/jokey/archive/2010/06/17/1759370.html
    */
    /*runregs = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows \
                        \\CurrentVersion\\Run",QSettings::NativeFormat); */
    runregs = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" \
                            ,QSettings::NativeFormat); //no UAC needed
    enableautoatart = new QCheckBox("开机自动启动");
    okbutton = new QPushButton("OK");
    applybutton = new QPushButton("Apply");
    cancelbutton = new QPushButton("Cacle");
    connect(cancelbutton,&QPushButton::clicked,this,&close);
    connect(applybutton,&QPushButton::clicked,this,&SetupWindow::execute_the_setup_apply);
    connect(okbutton,&QPushButton::clicked,this,&SetupWindow::execute_the_setup);

    QHBoxLayout *checkBoxLayout = new QHBoxLayout();
    checkBoxLayout->addWidget(enableautoatart);

    QHBoxLayout *buttonlayout = new QHBoxLayout();
    buttonlayout->addWidget(okbutton);
    buttonlayout->addWidget(applybutton);
    buttonlayout->addWidget(cancelbutton);

    QVBoxLayout *mainlayout = new QVBoxLayout();
    mainlayout->addLayout(checkBoxLayout);
    mainlayout->addLayout(buttonlayout);

    setLayout(mainlayout);

    //读取注册表项来初始化enableautoatart
    enableautoatart->setChecked(checkStartIteminRegEdit());
}

bool SetupWindow::checkStartIteminRegEdit()
{
    QVariant result;
    QString absoluteFilePath = QDir::currentPath()+"/networkforprism.exe";
    result=runregs->value("prismautostart",1024).toString();
    return result==absoluteFilePath;

}

bool SetupWindow::createStartIteminRegEdit()
{
    /*http://blog.csdn.net/ei__nino/article/details/7305082*/
    QString absoluteFilePath = QDir::currentPath();
    qDebug()<<absoluteFilePath;
    runregs->setValue("prismautostart",absoluteFilePath+"/networkforprism.exe");
}

bool SetupWindow::setStartIteminRegEdit(bool isenable)
{
    if(isenable) {
        if(!checkStartIteminRegEdit()) {
            createStartIteminRegEdit();
        }
    }
    else {
        runregs->remove("prismautostart");
    }
}

SetupWindow::~SetupWindow()
{
    delete enableautoatart;
    delete okbutton;
    delete applybutton;
    delete cancelbutton;
}

void SetupWindow::execute_the_setup()
{
    execute_the_setup_apply();
    close();
}

//这是存储的永久性数据，checkbox需要初始化
void SetupWindow::execute_the_setup_apply()
{
    bool autostart;
    autostart=enableautoatart->isChecked();
    enableautoatart->setDisabled(true);

    setStartIteminRegEdit(autostart);

    enableautoatart->setDisabled(false);
}
