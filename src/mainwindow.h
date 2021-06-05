#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <queue>
#include <QListWidget>

#define FIFO 0
#define LRU 1
#define UNEXECUTED 0 // 指令未执行
#define EXECUTED 1 // 指令已执行

const int INS_TOTAL = 320; // 每个作业指令总数
const int INS_PER_PAGE = 10; // 每个页面最多存放10条指令
const int PAGES = 4; // 每个作业分配4个页面

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class memory;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class memory;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QListWidget* pageList[PAGES];
};

class memory : public QObject
{
    Q_OBJECT
private:
    int insMem[PAGES][INS_PER_PAGE]; // 内存页面
    int insDisk[INS_TOTAL]; // 辅存页表
    int pageMissCount; // 缺页次数
    int pageFrequence[PAGES]; // 页使用频度
    int frequence, stopCode; // 频度、程序运行状态
    std::queue<int> q; // 队列，用于FIFO
    MainWindow* win;

    void executeIns(int, int); // 执行一条指令
    void _FIFO(int); // FIFO算法
    void _LRU(int); // LRU算法
    QString toQString(int); // 将指令地址转化为字符串

public:
    explicit memory(MainWindow*);
    ~memory(){}
    void init(); // 初始化
    void run(int); // 运行320条指令

private slots:
    void begin(int); // 开始执行

public slots:
    void stop(); // 停止执行
};

#endif // MAINWINDOW_H
