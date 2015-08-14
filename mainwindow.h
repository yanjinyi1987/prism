#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QtNetwork>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QSettings>
#include <QDebug>
namespace Ui {
class MainWindow;
}

namespace setupUi {
class SetupWindow;
}

class SetupWindow : public QDialog
{
    Q_OBJECT
public:
    SetupWindow();
    ~SetupWindow();
    void enable_auto_start();
    void disable_auto_start();
    QCheckBox *enableautoatart;
    QPushButton *okbutton;
    QPushButton *applybutton;
    QPushButton *cancelbutton;
    QSettings *runregs;


    bool checkStartIteminRegEdit();
    bool createStartIteminRegEdit();
    bool setStartIteminRegEdit(bool);

public slots:
    void execute_the_setup_apply();
    void execute_the_setup();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *) Q_DECL_OVERRIDE;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /*QSystemTrayIcon*/
    QSystemTrayIcon *myMsgIcon;
    void CreateTrayMenu();
    QAction *miniSizeAction,*maxSizeAction,*restoreWinAction,*quitAction,*setupAction;
    QMenu *myMenu;
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void systemsetup();

    bool islogin=false; /*是否已经登录了*/
    unsigned int detectcount=0;
    QTimer *timer;
    void setUserName(const QString &uname) //传递QString时的定义必须是这样，参见QString的构造函数
    {
        username=uname;
    }

    void setPasswd(const QString &passwd)
    {
        password=passwd;
    }

    QString &getUserName()
    {
        return username;
    }

    QString &getPasswd()
    {
        return password;
    }

    QByteArray gen_postdata();
    void sendRequest();
    void replyFinished(QNetworkReply *);

private:
    Ui::MainWindow *ui;
    QString username,password;
    QNetworkAccessManager *manager;
};

#endif // MAINWINDOW_H
