#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QChartView>       //图表组件头文件
#include <QTimer>           //定时器对象头文件
#include <QLineSeries>      //折线对象头文件
#include <QValueAxis>       //坐标轴对象头文件
#include <QFile>
#include <QTextStream>
#include <QString>

extern "C"{
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/input.h"
}

//添加图表命名空间
QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:

    //声明LED开关，蜂鸣器开关，显示小组信息，手动更新按钮
    QPushButton *btn_led,*btn_beep,*btn_grp_info,*btn_man_update;

    //声明信息窗口，显示数据
    QTextEdit *info_window;

    //声明图表
    QChart *chart;

    //声明图表视图
    QChartView *chartView;

    int maxX;                   //x轴最大值
    int maxY;                   //y轴最大值
    QValueAxis *axisY;          //Y轴
    QValueAxis *axisX;          //X轴
    QLineSeries *lineSeries;    //折线
    QList<float> data;          //浮点数据QList
    int cnt;                    //当前坐标x

    //声明组件容器
    QWidget *widg_menu,*widg_window;

    //声明窗口布局
    QVBoxLayout *layout_menu;//左侧菜单布局
    QVBoxLayout *layout_window;//右侧窗口布局

    //声明数据来源
    QFile *src_data;

    //声明状态量（蜂鸣器、灯光、小组信息）
    bool status_beep,status_led,status_grpinfo,status_update,status_auto;
    QString grp_info;//
    int num_info;//显示行数
    QTimer *timer;//定时器
    int fd;//文件标识
    QString string;//窗口信息
    QString update_info;//更新信息

    //声明文件大小
    int size;

    void beep_event(int fd, int value);

public slots:
    void led_toggle();//开关灯
    void beep_toggle();//开关蜂鸣器
    void show_grpinfo();//显示组员信息
    void switch_manual();//切换手动模式
    void updateInfo();//更新信息和图表

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


};
#endif // MAINWINDOW_H
