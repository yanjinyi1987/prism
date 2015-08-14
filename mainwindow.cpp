#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>
#include <QEvent>
#include <QNetworkReply>
#include <QTextCodec>
#include <QRegExp>
#include <QWebElement>
#include <QWebFrame>
#include <QWebPage>

#include <QMessageBox>
#include <QAction>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*设置图标和标题*/
    setWindowTitle("Check PRISM");
    QIcon prismicon=QIcon(":/windowlogo/logo");
    setWindowIcon(prismicon);

    /*QSystemTrayIcon test 这货必须是public的，否则，别的地方怎么使用它啊*/
    if(QSystemTrayIcon::isSystemTrayAvailable()) {
        myMsgIcon = new QSystemTrayIcon();
        myMsgIcon->setIcon(prismicon);
        connect(myMsgIcon,&QSystemTrayIcon::activated,this,&MainWindow::iconActivated);
        CreateTrayMenu();
        myMsgIcon->show();
    }

    /*设置Timer*/
    timer=new QTimer(this);
    timer->start(3600*1000); //check per hour
    manager = new QNetworkAccessManager(this); //新建QNetworkAccessManager对象
    connect(manager,&QNetworkAccessManager::finished,this,&MainWindow::replyFinished);//关联信号和槽
    connect(ui->refreshbutton,&QPushButton::clicked,this,&MainWindow::sendRequest);
    connect(timer,&QTimer::timeout,this,&MainWindow::sendRequest);
    //intial Request
    //new20150703 如果网络连接不通，sendRequest如何动作，QNetworkAccessManager是如何作为中间层处理这中问题的
    sendRequest();
}

MainWindow::~MainWindow()
{
    delete miniSizeAction;
    delete maxSizeAction;
    delete restoreWinAction;
    delete quitAction;
    delete setupAction;

    delete myMsgIcon;
    delete timer;
    delete ui;
}

void MainWindow::sendRequest()
{ 
    //disable timer and refreshbutton
    MainWindow::detectcount++; //global
    qDebug()<<QString::fromLatin1("we have detected %1 times!").arg(MainWindow::detectcount);
    ui->timercount->setText(QString::fromLatin1("we have detected %1 times!").arg(MainWindow::detectcount));
    ui->timercount->adjustSize(); //自动调整大小
    /*http://blog.const.net.cn/a/12615.htm
     *LabelName->setGeometry(QRect(328, 240, 329, 27*4));  //四倍行距
     *LabelName->setWordWrap(true);
     *LabelName->setAlignment(Qt::AlignTop);
     */
    //显示logo，获取默认背景色并传递给字符串
    QPalette currentPalette=QPalette();
    QColor bcolor=currentPalette.color(QPalette::Window);
    QString sheetforWaiting=QString::fromLatin1("color:black;background-color:%1").arg(bcolor.name());
    ui->current_status->setStyleSheet(sheetforWaiting);
    ui->current_status->setText("Waiting Loading...");

    //manager->get(QNetworkRequest(QUrl("https://prism.osapublishing.org/Author")));//发送请求
    /*
     * name="UserName"
     * name="Password"
    */
    //QNetworkReply * QNetworkAccessManager::post(const QNetworkRequest & request, const QByteArray & data)
    //QNetworkRequest request1 = QNetworkRequest(QUrl("https://prism.osapublishing.org/Account/Login?ReturnUrl=%2FAuthor"));
    //自己构造headers
    QNetworkRequest request1;
    QByteArray postdata=gen_postdata();
    int contentlength=postdata.length();
    request1.setUrl(QUrl("https://prism.osapublishing.org/Account/Login?ReturnUrl=%2FAuthor"));
    request1.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    request1.setHeader(QNetworkRequest::ContentLengthHeader,contentlength);

    manager->post(request1,postdata);
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    //http redirection
    int statuscode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    switch(statuscode) {
    case 200: //OK
    {
        QTextCodec *codec;
        QByteArray rawall=reply->readAll();
        QString utf8all;
        codec=QTextCodec::codecForName("utf8"); //使用utf8编码，以显示多字节字符
        utf8all=codec->toUnicode(rawall);
        //ui->debugBrowser->setText(utf8all);
        //ui->obj_status->setText(utf8all);
        //获取http body的数据，具体的是提取一个状态 即 Corresponding Author下的Process,这里涉及到如何提取和解析table的问题
        //qDebug()<<rawall;
/*
        //get tabname
        QString tabname;
        QString pattern_tabname="<a href=\"([^\"]+)\" class=\"[^\"]*\">Corresponding Author</a>";
        QRegExp rx(pattern_tabname);
        int pos=rx.indexIn(rawall);
        qDebug()<<pos;
        if(pos>-1) {
            tabname=rx.cap(1);//"tabs-1";
        }
        qDebug()<<tabname;
        //使用正则表达式 解析div和
        QString pattern="<div id="+tabname+">"+"</div>";
        QRegExp rx(pattern);
        rx.setCaseSensitivity(cs);
*/ //我们不能用正则表达式来解析html文本见 http://wiki.qt.io/Handling_HTML
        //get tabname
        QString tabname;
        QString pattern_tabname="<a href=\"([^\"]+)\" class=\"[^\"]*\">Corresponding Author</a>";
        QRegExp rx(pattern_tabname);
        int pos=rx.indexIn(rawall);
        qDebug()<<pos;
        if(pos>-1) {
            tabname=rx.cap(1);//"tabs-1";
        }
        qDebug()<<tabname;
        QWebPage *page=new QWebPage();
        page->mainFrame()->setHtml(rawall);
        QWebElement parse=page->mainFrame()->documentElement();
        QString mydivcss="#"+tabname.remove(0,1); //http://blog.csdn.net/y296144646q/article/details/6074572
        QWebElement mydiv=parse.findFirst(mydivcss);
        //qDebug()<<mydiv.toPlainText();
        //the data we wanted is in the toPlainText();
        QString mydata=mydiv.toPlainText();
        QWebElement datatable=mydiv.firstChild();
        QWebElement datahead=datatable.firstChild();
        QWebElement databody=datahead.nextSibling();
        qDebug()<<datahead.toPlainText();
        qDebug()<<databody.toPlainText();
        QStringList headstr=datahead.toPlainText().split("\n");
        QStringList bodystr=databody.toPlainText().split("\n");
        headstr.removeLast();
        bodystr.removeLast();
        QString currentstatus=bodystr.last();

        QString sheetforPeerReview=QString::fromLatin1("color:rgb(255,170,0);background-color:rgb(0,0,127)");
        if(currentstatus=="Peer Review") {
            ui->current_status->setStyleSheet(sheetforPeerReview);
        }
        ui->current_status->setText(currentstatus);
        /*如果检测到程序是最小化状态或者是非活动状态，那么需要设置额外的提醒比如响铃，不过现在貌似不流行弹窗了*/
        if(isActiveWindow()) {
            //闪动图标
            ;
        }
        //插入自定义的信号来enablerefreshbutton和时钟
        reply->deleteLater(); //最后要释放reply对象
        break;
    }
    case 302: //redirection
    {
        qDebug()<<statuscode;
        QVariant redi_location=reply->header(QNetworkRequest::LocationHeader);
        reply->deleteLater(); //最后要释放reply对象
        QUrl redi_url=redi_location.toUrl();
        qDebug()<<redi_url;
        QNetworkRequest request_redi;
        request_redi.setUrl(redi_url);
        manager->get(request_redi);
        break;
    }
    default:
    {
        break;
    }
    }
}

QByteArray MainWindow::gen_postdata()
{
    setUserName("yanjinyi@mail.sim.ac.cn");
    setPasswd("153381523");
    QString postdata="";
    QString name1="UserName",name2="Password";
    QString value1=username,value2=password;
    postdata+=name1+"="+value1+"&"+name2+"="+value2;
    return postdata.toUtf8();
}

/*添加开机启动功能*/
/*最小化到托盘并且利用Timer定时弹出气泡提示*/
/*
 * QSystemTrayIcon
 *
*/
/*http://blog.csdn.net/qustdjx/article/details/20283863*/
void MainWindow::changeEvent(QEvent *e)
{/*showMinimized貌似不会通知你状态的改变*/
    qDebug()<<"outside"<<e;
    if(e->type()==QEvent::WindowStateChange) {
        qDebug()<<"inside"<<windowState();
        switch (QMainWindow::windowState()) {
        case Qt::WindowMinimized:
            myMsgIcon->setVisible(true);
            myMsgIcon->showMessage("信息","我已经被创建啦!");
            break;
        case Qt::WindowNoState:
        case Qt::WindowMaximized:
        case Qt::WindowFullScreen:
            myMsgIcon->showMessage("信息","我恢复原状啦!");
            myMsgIcon->setVisible(false);
            break;
        case Qt::WindowActive:
            break;
        default:
            myMsgIcon->setVisible(true); //会出现QFlags(0x1|0x2)的状况哦！
            myMsgIcon->showMessage("信息","我已经被创建啦!");
        }
    }
}
//关闭到托盘
void MainWindow::closeEvent(QCloseEvent *e)
{
    e->ignore();
    hide(); //最小化并隐藏窗口
    if(myMsgIcon->isVisible()==false) {
        myMsgIcon->setVisible(true);
        myMsgIcon->showMessage("信息","程序已经最小化，请从任务栏退出，谢谢！");
    }
}
void MainWindow::systemsetup()
{
    //生成一个设置界面来对系统注册表进行写入和删除已实现是否开机启动
    //这里我需要手写一个界面么？
    SetupWindow newsetupwindow;
    newsetupwindow.exec();

}

void MainWindow::CreateTrayMenu()
{
    miniSizeAction = new QAction("最小化(&N)",this);
    maxSizeAction = new QAction("最大化(&X)",this);
    restoreWinAction = new QAction("还 原(&R)",this);
    quitAction = new QAction("退出(&Q)",this);
    setupAction = new QAction("设置(&S)",this);

    connect(miniSizeAction,&QAction::triggered,this,&MainWindow::hide);
    connect(maxSizeAction,&QAction::triggered,this,&MainWindow::showMaximized);
    connect(restoreWinAction,&QAction::triggered,this,&MainWindow::showNormal);
    connect(quitAction,&QAction::triggered,this,&QApplication::quit);
    connect(setupAction,&QAction::triggered,this,&MainWindow::systemsetup);

    myMenu = new QMenu((QWidget *)QApplication::desktop());

    myMenu->addAction(miniSizeAction);
    myMenu->addAction(maxSizeAction);
    myMenu->addAction(restoreWinAction);
    myMenu->addSeparator();
    myMenu->addAction(setupAction);
    myMenu->addSeparator();
    myMenu->addAction(quitAction);

    myMsgIcon->setContextMenu(myMenu);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:
        hide();
        break;
    case QSystemTrayIcon::DoubleClick:
        showNormal();
        MainWindow::activateWindow();
        break;
    case QSystemTrayIcon::MiddleClick:
        myMsgIcon->showMessage("信息","点什么点",QSystemTrayIcon::Information,5000);
        break;
    default:
        break;
    }
}
