# 内存管理（请求调页式）

## 目录

  * [项目需求](#项目需求)
  * [开发环境](#开发环境)
  * [项目结构](#项目结构)
  * [操作说明](#操作说明)
  * [项目实现](#项目实现)
    * [状态设计](#状态设计)
    * [类设计](#类设计)
    * [模拟运行320条指令](#模拟运行320条指令)
    * [执行一条指令](#执行一条指令)
    * [FIFO替换算法](#fifo替换算法)
    * [LRU替换算法](#lru替换算法)

## 项目需求

1. 假设每个页面可存放10条指令，分配给一个作业的内存块为4页。模拟一个作业的执行过程，该作业有**320条指令**，即它的地址空间为32页，目前所有页还没有调入内存。
2. 在模拟过程中，如果所访问指令在内存中，则显示其物理地址，并转到下一条指令；如果没有在内存中，则发生缺页，此时需要记录**缺页次数**，并将其调入内存。如果4个内存块中已装入作业，则需进行页面置换。
3. 所有320条指令执行完成后，计算并显示作业执行过程中发生的**缺页率**。
4. 置换算法可以选用**FIFO**或者**LRU**算法。
5. 50%的指令是**顺序执行**的，25%是均匀分布在**前地址部分**，25％是均匀分布在**后地址部分**。

## 开发环境

- **操作系统**：Windows10
- **开发软件**：
  1. Qt  *v5.15.2*
  2. Qt Creator *v4.15.0* (Community)
- **开发语言**：C++
- **主要引用模块**：
  1. Qt核心组件: QObject, QMainWindow, QCoreApplication, QTime
  2. QtWidgets: QLabel, QComboBox,  QPushButton, QListWidget
  3. STL标准库: queue

## 项目结构

```
│  README.md
│
└─src
   │  OS_Memory.pro
   │  logo.ico
   │
   ├─Headers
   │     mainwindow.h
   │  
   └─Sources
         main.cpp
         mainwindow.cpp
```

## 操作说明

- 双击`OS_Memory`，进入内存管理模拟系统。

  <img src="img\test1.png" width = "600" alt="test1" align=center />

- 下拉框选择`FIFO`或`LRU`算法，点击`开始`进行模拟。

  <img src="img\test2.png" width = "600" alt="test2" align=center />

- 运行结束后选择缺页总数和缺页率。

  <img src="img\test3.png" width = "600" alt="test3" align=center />

## 项目实现

### 状态设计

1. 算法类型：
   - `#define FIFO 0`
   - `#define LRU 1`
2. 指令状态：
   - `#define UNEXECUTED 0`  // 指令未执行
   - `#define EXECUTED 1` // 指令已执行
3. 其他参数：
   - `const int INS_TOTAL = 320; `// 每个作业指令总数
   - `const int INS_PER_PAGE = 10;` // 每个页面最多存放10条指令
   - `const int PAGES = 4;` // 每个作业分配4个页面

### 类设计

```c++
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
    void replace(int, int); // 替换页面
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
```

### 模拟运行320条指令

1. 从320指令**中间开始执行**，即先执行地址为159的指令。然后**执行下一条**地址为160的指令。
2. 在地址为**000~158**的指令中，执行一条未被执行的指令。若此指令的下一条指令也未被执行，再**执行下一条**指令。
3. 在地址为**160~319**的指令中，执行一条未被执行的指令。若此指令的下一条指令也未被执行，再**执行下一条**指令。
4. 继续执行第2步，直到320条指令都被执行完毕。

通过以上步骤，在前地址部分和后地址部分分别执行两条连续的指令，可以保证所有条指令中，有50顺序执行，25%跳转到前地址执行，25%跳转到后地址执行。

```c++
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
```

### 执行一条指令

1. 每执行一条指令，就显示此指令地址信息，并把频度`frequence`加一。
2. 遍历内存中4张页表的每条指令地址，模拟**查找指令**的过程。
   - 如果在某页中**找到指令**，就将此页对应的频度`pageFrequence[pageFound]`更新。
   - 如果**没找到指令**，就调用**替换算法**进行调页（FIFO或LRU）。同时将调换的页对应的频度``pageFrequence[pageFound]`更新（在替换算法的函数中实现）。

```c++
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
        
        win->ui->pagemissCountLabel->setText(QString::number(pageMissCount));
    }
    else // 如果找到
    {
        //cout << "instruction found in page " << pageFound <<endl;
        pageFrequence[pageFound] = frequence;
    }

    Sleep(150);
}
```

### FIFO替换算法

1. 选择需要替换的内存页面
   - 如果有内存页面为空，则将空页面设位待替换页面。
   - 否则，从队列`q`中弹出一个页面，此页面为待替换页面。
2. 计算辅存中代替换页面，进行页面替换。
3. 将刚替换过的内存页面压入队列`q`。

```c++
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
    pageFrequence[pageMem] = frequence；
        
    replace(pageMem, pageDisk); //模拟将辅存页面调入内存页面
    q.push(pageMem);
}
```

### LRU替换算法

1. 选择页面使用频度`pageFrequence[i]`最小的页面为待替换内存页面，即最近最少使用页面。
2. 计算辅存中待替换页面，进行页面替换。

```c++
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
```

