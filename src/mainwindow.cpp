#include "mainwindow.h"
#include "memory.h"
#include "ui_mainwindow.h"
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pageList[0] = ui->listWidget_1;
    pageList[1] = ui->listWidget_2;
    pageList[2] = ui->listWidget_3;
    pageList[3] = ui->listWidget_4;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void Sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}


memory::memory(MainWindow* w) : QObject(w)
{
    stopCode = 0;
    win = w;
    init();

    connect(win->ui->beginButton, &QPushButton::clicked, this, [=]()
    {
        memory::begin(win->ui->algorithmBox->currentIndex());
    });
}

void memory::init()
{
    int i, j;
    while(!q.empty()) q.pop();
    frequence = 0;
    pageMissCount = 0;
    srand((unsigned)time(NULL));//设置随机种子


    //初始化内存页面
    for(i = 0; i < PAGES; i++)
        for(j = 0; j < INS_PER_PAGE; j++)
        {
            insMem[i][j] = -1;
            //item[i][j]->setData(QString::number(-1));
            win->pageList[i]->item(j)->setData(0, " ");
        }

    //初始化辅存页表
    for(i = 0; i < INS_TOTAL; i++)
        insDisk[i] = UNEXECUTED;

    //初始化页使用频度
    for(i = 0; i < PAGES; i++)
        pageFrequence[i] = 0;
}

void memory::stop()
{
    stopCode = 1;
}

void memory::run(int algorithm)
{
    int frontCount = 0, endCount = 0, ins;

    //从中间开始执行
    ins = INS_TOTAL / 2 - 1;
    executeIns(ins, algorithm);
    //执行下一条指令
    executeIns(ins + 1, algorithm);

    int frontIns = ins; // 前地址部分指令总数
    int endIns = INS_TOTAL - ins - 2; // 后地址部分指令条数

    while(frontCount < frontIns || endCount < endIns)
    {
        if(stopCode == 1) return;

        //前地址部分找到一条指令
        while(1)
        {
            if(stopCode == 1) return;

            if(frontCount == frontIns) break;
            ins = rand() % frontIns;
            if(insDisk[ins] == UNEXECUTED) // 找到未执行指令
            {
                executeIns(ins, algorithm);
                frontCount++;
                break;
            }
        }

        //执行下一条
        ins++;
        if(insDisk[ins] == UNEXECUTED)
        {
            executeIns(ins, algorithm);
            frontCount++;
        }

        //后地址部分找到一条指令
        while(1)
        {
            if(stopCode == 1) return;

            if(endCount == endIns) break;
            ins = rand() % endIns + frontIns + 2;
            if(insDisk[ins] == UNEXECUTED) // 找到未执行指令
            {
                executeIns(ins, algorithm);
                endCount++;
                break;
            }
        }

        //执行下一条
        ins++;
        if(ins < INS_TOTAL && insDisk[ins] == UNEXECUTED)
        {
            executeIns(ins, algorithm);
            endCount++;
        }
    }
    //计算缺页率
    double pageMissRate = (pageMissCount / (double)INS_TOTAL) * 100;

    win->ui->pagemissCountLabel->setText(QString::number(pageMissCount)); // 显示缺页总数
    win->ui->pagemissRateLabel->setText(QString::number(pageMissRate) + "%"); // 显示缺页率
}

void memory::executeIns(int ins, int algorithm)
{
    win->ui->addressLabel->setText(toQString(ins)); // 显示此指令地址信息

    frequence++; // 执行一条指令，就使频度数加一
    int pageFound = -1, i, j;

    //模拟在内存页中查找指令
    for(i = 0; i < PAGES; i++)
    {
        for(j = 0; j < INS_PER_PAGE; j++)
            if(insMem[i][j] == ins)
            {
                pageFound = i;
                break;
            }
        if(pageFound != -1) break;
    }

    if(pageFound == -1) // 如果没找到, 发生缺页
    {
        //cout << "page miss, ";

        pageMissCount++;
        if(algorithm == LRU) // 使用LRU替换算法
            _LRU(ins);
        else if(algorithm == FIFO) // 使用FIFO替换算法
            _FIFO(ins);

        win->ui->pagemissCountLabel->setText(QString::number(pageMissCount)); // 显示缺页总数
    }
    else // 如果找到
    {
        //cout << "instruction found in page " << pageFound <<endl;
        pageFrequence[pageFound] = frequence;
    }

    Sleep(150);
}

void memory::_FIFO(int ins)
{
    int i, pageMem = -1, pageDisk;

    //查找是否有内存页为空
    for(i = 0; i < PAGES; i++)
        if(pageFrequence[i] == 0) // 如果有内存页为空
        {
            pageMem = i;
            break;
        }

    if(pageMem == -1) // 如果没有内存页为空
    {
        pageMem = q.front();
        q.pop();
    }
    //cout << "replace page " << pageMem << " in memory" <<endl;

    pageDisk = ins / INS_PER_PAGE; // 计算辅存中对应的页面
    pageFrequence[pageMem] = frequence;

    replace(pageMem, pageDisk); //模拟将辅存页面调入内存页面
    q.push(pageMem);
}

void memory::_LRU(int ins)
{
    int i, pageMem = 0, pageDisk, lastest = pageFrequence[0];
    for(i = 1; i < PAGES; i++)
        if(pageFrequence[i] < lastest)
        {
            lastest = pageFrequence[i];
            pageMem = i;
        }
    //cout << "replace page " << pageMem << " in memory" <<endl;

    pageDisk = ins / INS_PER_PAGE; // 计算辅存中对应的页面
    pageFrequence[pageMem] = frequence;

    replace(pageMem, pageDisk);//模拟将辅存页面调入内存页面
}

void memory::replace(int pageMem, int pageDisk)
{
    int i, address;
    for(i = 0; i < INS_PER_PAGE; i++)
    {
        address = (pageDisk * INS_PER_PAGE) + i;
        insMem[pageMem][i] = address;
        win->pageList[pageMem]->item(i)->setData(0, "          " + toQString(address));
    }
}

QString memory::toQString(int ins)
{
    int n = 3, i, temp = ins;
    if(ins == 0) n--;
    while(temp > 0)
    {
        temp /= 10;
        n--;
    }
    QString s;
    for(i = 0; i < n; i++)
        s += "0";
    s += QString::number(ins);
    return s;
}

void memory::begin(int algorthim)
{
    win->ui->beginButton->setDisabled(true);
    win->ui->pagemissCountLabel->setText("");
    win->ui->pagemissRateLabel->setText("");
    win->ui->addressLabel->setText("");
    init();
    run(algorthim);
    win->ui->beginButton->setEnabled(true);
}
