#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /********************实例化********************/

    //实例化按钮
    btn_led=new QPushButton();
    btn_beep=new QPushButton();
    btn_grp_info=new QPushButton();
    btn_man_update=new QPushButton();

    //实例化窗口
    info_window=new QTextEdit();

    //实例化图表
    chart=new QChart();

    //实例化图表视图
    chartView = new QChartView();

    //实例化其他变量
    maxX=51;//最大显示数据量51个
    maxY=31;//测量最大温度为28度
    axisX = new QValueAxis();           //X轴
    axisY = new QValueAxis();           //Y轴
    lineSeries =  new QLineSeries();    //折线

    //实例化组件容器
    widg_menu=new QWidget(this);
    widg_window=new QWidget(this);

    //实例化窗口布局
    layout_menu=new QVBoxLayout();
    layout_window=new QVBoxLayout();

    //实例化数据来源
    src_data=new QFile("/newdata.csv");

    //实例化状态量和小组信息
    status_beep=0;
    status_led=0;
    status_grpinfo=1;
    status_update=0;
    status_auto=1;
    num_info=0;
    grp_info="林永乐、卜鑫彬";
    update_info="AutoUpdate";
    timer=new QTimer(this);
    timer->start(500);
    size=0;
    cnt=0;



    /****************设计布局**************/

    /* 设置主窗口的位置与大小 */
    this->setGeometry(0, 0, 800, 480);

    widg_menu->setGeometry(0, 0, 100, 480);
    widg_window->setGeometry(100, 0, 700, 480);

    //设置菜单的布局
    widg_menu->setLayout(layout_menu);

    btn_led->setText("LED");
    btn_led->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    layout_menu->addWidget(btn_led);

    btn_beep->setText("BEEP");
    btn_beep->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    layout_menu->addWidget(btn_beep);

    btn_grp_info->setText("组员信息");
    btn_grp_info->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    layout_menu->addWidget(btn_grp_info);

    btn_man_update->setText("手动更新");
    btn_man_update->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    layout_menu->addWidget(btn_man_update);

    layout_menu->setSpacing(10);

    //设置右侧窗口布局
    widg_window->setLayout(layout_window);

    info_window->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    layout_window->addWidget(info_window);

    chart->legend()->hide();            // 多折线的区分组件，本例只有一条，可以把这组件隐藏起来
    chart->setTitle("温度数据");         //设置标题
    chart->addSeries(lineSeries);       //添加折线
    axisY->setLabelFormat("%i");            //Y轴显示格式
    axisY->setTitleText("温度");            //Y轴名称
    chart->addAxis(axisY,Qt::AlignLeft);    //添加Y轴方向、位置
    axisY->setRange(26,maxY);                //Y轴范围
    lineSeries->attachAxis(axisY);          //折线附着Y轴

    axisX->setLabelFormat("%i");            //X轴显示格式
    axisX->setTitleText("次数");            //X轴名称
    chart->addAxis(axisX,Qt::AlignBottom);  //添加X轴方向、位置
    axisX->setRange(0,maxX);                //X轴范围
    lineSeries->attachAxis(axisX);          //折线附着X轴

    chartView->setChart(chart);             //将图表设置在图表视图中
    chartView->setRenderHint(QPainter::Antialiasing);   //抗锯齿

    chartView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    layout_window->addWidget(chartView);

    //绑定信号和槽函数
    connect(btn_led,SIGNAL(clicked()),this,SLOT(led_toggle()));
    connect(btn_beep,SIGNAL(clicked()),this,SLOT(beep_toggle()));
    connect(btn_grp_info,SIGNAL(clicked()),this,SLOT(show_grpinfo()));
    connect(btn_man_update,SIGNAL(clicked()),this,SLOT(switch_manual()));
    connect(timer,SIGNAL(timeout()),this,SLOT(updateInfo()));


}

MainWindow::~MainWindow()
{
}

void MainWindow::led_toggle(){
    status_led=!status_led;
    fd=::open("/dev/myled", O_RDWR);
    ::write(fd,&status_led,sizeof(status_led));
    ::close(fd);
}

void MainWindow::beep_toggle(){
    status_beep=!status_beep;
    fd=::open("/dev/input/event3", O_WRONLY);
    beep_event(fd,status_beep);
    ::close(fd);
}

void MainWindow::show_grpinfo(){
    status_grpinfo=!status_grpinfo;
}

void MainWindow::switch_manual(){
    status_auto=!status_auto;
}

void MainWindow::updateInfo(){
    int endData;
    src_data->open(QIODevice::ReadWrite|QIODevice::Text);
    QTextStream dataStream(src_data);
    QList<QString>rows;
    while(!dataStream.atEnd()){
        QString line = dataStream.readLine();   //分行读取数据
        rows<<line.split(',');                  //分割出代表数据的字符串
    }
    if(cnt==maxX)
    {
        lineSeries->clear();                //清除折线上的数据（如有的话）
        cnt=0;
        /*
        size=0;
        src_data->close();
        src_data->open(QIODevice::ReadWrite|QIODevice::Truncate);
        src_data->flush();
        */
    }
    src_data->close();

        if(rows.size()>size||num_info==0){
            if(rows.size()>size)
                status_update=1;
            size=rows.size();
            for(int i=0;i<rows.size();i++){
                float tmp=rows.at(i).toFloat();
                data.append(tmp);    //字符串转换成浮点形保存到data列表中
                if(tmp>30)
                    ::printf("%f,too hot!!!!\n",tmp);
            }



            endData=data.size();
            if(data.size()>0){
                cnt = cnt%maxX;     //计算x轴每个数据之间的间隔：最大值/
                lineSeries->append(cnt,data.at(endData-1));
                cnt++;
            }
        }

    if(status_update==0){
        update_info="NoUpdate";
        if(status_auto==0)
            return;
    }
    else{
        status_update=0;
        if(status_auto){
            update_info="AutoUpdate ";
            update_info.append(rows.at(rows.size()-1));
        }
        else{
            update_info="ManualUpdate ";
            update_info.append(rows.at(rows.size()-1));
        }
    }

    if(num_info==20){
        num_info=0;
        string.clear();
    }
    QTextStream textStream(&string);
    textStream.setFieldAlignment(QTextStream::AlignLeft);
    if(num_info==0){
        textStream<<qSetFieldWidth(15)<<QString("蜂鸣器")
                 <<qSetFieldWidth(15)<<QString("LED")
                <<qSetFieldWidth(30)<<QString("小组信息")
               <<qSetFieldWidth(15)<<QString("更新信息")
              <<endl;
    }
    num_info++;
        if(status_grpinfo){
            textStream<<qSetFieldWidth(15)<<status_beep
                     <<qSetFieldWidth(15)<<status_led
                    <<qSetFieldWidth(21)<<grp_info
                   <<qSetFieldWidth(15)<<update_info
                  <<endl;
        }
        else{
            textStream<<qSetFieldWidth(15)<<status_beep
                     <<qSetFieldWidth(60)<<status_led
                   <<qSetFieldWidth(15)<<update_info
                  <<endl;
        }

    info_window->setPlainText(textStream.readAll());
}

void MainWindow::beep_event(int fd, int value){
    fd=::open("/dev/input/event3", O_WRONLY);

    struct input_event ev;
    ::memset(&ev, 0, sizeof(ev));
    ev.type = EV_SND;
    ev.code = SND_BELL;
    ev.value = value;
    ::write(fd, &ev, sizeof(ev));

    ::memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    ::write(fd, &ev, sizeof(ev));

    ::close(fd);
}
