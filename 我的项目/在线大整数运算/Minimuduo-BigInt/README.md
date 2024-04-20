# 在线大整数运算<BigInt>
##### 介绍：
    提供大整数(万位以上，主要受硬件资源限制)的在线快速运算，主解决本地计算机不能进行超大数 据运算的问题，采用B/S架构。
    
##### 后台服务器部分： 
    1.实现大整数的基本运算：Add，Sub，Mul，Div，Mod，Square，Pow，PowMod


##### Web服务部分(webserver)
    参考陈硕的muduo实现了一个MiniMuduo，在此基础上实现webserver
  
#####  webserver详细介绍：

* 采用one loop per thread + threadpool(计算线程池) 的并发服务器模型
* 使用状态机解析HTTP请求报文，支持解析GET和POST请求
* 访问数据库(mysql)，实现web端的用户注册也登录功能，可以请求服务器图片
* 采用异步日志类，来记录服务器的运行状态
* 经Webbench压力测试可以实现上万的并发连接
    

## MiniMuduo核心技术点
  
##### Base基础库(MiniMuduo/Base)
**Exception类**
   异常类的封装，对外提供what()输出错误信息，stacktrace()函数进行栈追踪，使用时需要throw muduo::Exception("oops"),外部使用catch(const muduo::Exception &ex)捕获并使用ex.what()/stacktrace()获取详细信息

**TimeStamp类**
   TimeStamp类继承至less_than_comparable<>,只需要实现<,可自动实现>,<=,>=

**Atomic类**
   原子性操作比锁的开销小，所以我们可以使用gcc提供的自增自减原子操作；最小的执行单元是汇编语句，不是语言语句
**CountDownLatch类**
   既可以用于所有子线程等待主线程发起 "起跑"，也可以用于主线程等待子线程初始化完毕才开始工作。其中使用RAII技法封装MutexLockGuard,holder表示锁属于哪一个线程。(注意holder是MutexLock的成员)
**BlockingQueue**

BlockingQueue和BoundedBlockingQueue分别是无界有界队列，本质上是生产者消费者问题，使用信号量或者条件变量解决。ThreadPool的本质也是生产者消费者问题，任务队列中是任务函数（生产者），线程队列就相当于消费者。
**异步日志类**

* 对于一般的日志类的实现，(1) 重载<<格式化输出 (2)级别处理 (3)缓冲区。
* 为了提高效率并防止阻塞业务线程，用一个背景线程负责收集日志消息，并写入日志文件，其他业务线程只管往这个日志线程发送日志消息，这称为异步日志。基本实现仍然是生产者（业务线程）与消费者（日志线程）和缓冲区，但是这样简单的模型会造成写文件操作比较频繁，因为每一次signal我们就需要进行写操作，将消息全部写入文件，效率较低。muduo使用多缓冲机制，即mutliple buffering，使用多个缓冲区，当一块缓冲区写满或者时间超时才signal；如果发生消息堆积，会丢弃只剩2块内存块。另外使用swap公共缓冲区来避免竞争，一次获得所有的消息并写入文件。
* 
***核心逻辑**
（1）前端业务线程将日志写到currentBuffer_，nextBuffer_是后备的一块缓冲区，buffers_是要写入到日志文件的缓冲区列表；
（2）currentBuffer_写满后，添加到buffers_，会通知后端的日志线程写日志文件，并且currentBuffer_会指向nextBuffer_所指向的缓冲区，currentBuffer_不写满，后端线程是不会将数据写入到日志文件中的。
（3）后端日志线程得到通知后，此时前端的currentBuffer_没有满，也会将其添加到buffers_中；将buwBuffer1_缓冲区给currentBuffer_，将newBuffer2_缓冲区给nextBuffer_，保证前端的业务线程还能写日志消息到缓冲区中。
（4）当buffer_缓冲区的数据写完之后，还需要预留2块缓冲区给newBuffer1_和newBuffer2_

##### 双缓冲的优点：使得前端的业务线程与后端的日志线程能够并发，且写日志不太频繁，提高了效率。


##### Base网络库(MiniMuduo/net)

* **reactor**
reactor+线程池适合CPU密集型，multiple reactors适合突发I/O型，一般一个千兆网口一个rector；multiple rectors（线程） + thread pool 更能适应突发I/O和密集计算。其中multiple reactors中的
mainReactor关注的是acceptor，也就是监听socket所关注的事件；
subReactor关注的是已连接socket所关注的事件，每次新到一个连接，就选择一个subReactor来处理该连接（也就选择了该Reactor所对应的线程来处理连接）；多个sub reactor采用round-robin的机制分配。

* **TimerQueue类**
 timers_和activeTimers_保存的是相同的数据，timers_是按到期时间排序，activeTimers_按照对象地址排序，并且timerQueue只关注最早的那个定时器，所以当发生可读事件的时候，需要使用getExpired()获取所有的超时事件，因为可能有同一时刻的多个定时器。


**TcpConnection类**

TcpServer还包含了一个TcpConnection列表这是一个已连接列表。TcpConnection与Acceptor类似，有两个重要的数据成员，Socket与Channel。所以说，Acceptor用于accept接受TCP连接。Acceptor的数据成员包括Socket、Channel，Acceptor的socket是listening socket。Channel用于观察此socket的readable事件，并回调Acceptor::handleRead(),后者调用accept来接受新连接，并回调TcpServer::newConnection callback。

**连接建立时序图**


![86f507a2e4c876c98c64eff17c9f1c2b.png](en-resource://database/513:1)

**连接断开时序图**


![4b812c76891f54286ca0096568934e4c.png](en-resource://database/515:1)


**Buffer类的设计**


对外表现为一块连续的内存(char*, len)，以方便客户代码的编写。其 size() 可以自动增长，以适应不同大小的消息。它不是一个 fixed size array (即 char buf[8192])，通过vector自动实现利用临时栈上空间，如果读入的数据不多，那么全部都读到 Buffer 中去了；如果长度超过 Buffer 的 writable 字节数，就会读到栈上的 stackbuf 里，然后程序再把 stackbuf 里的数据 append 到 Buffer 中。


readv()将文件中连续的数据块读入内存分散的缓冲区中。writev()收集内存中分散的若干缓冲区中的数据写至文件的连续区域中。#include <sys/uio.h> ssize_t readv(int fildes, const struct iovec *iov, int iovcnt);ssize_t writev(int fildes, const struct iovec *iov, int iovcnt);


#####  **runInLoop**
**runInLoop的实现**：需要使用eventfd唤醒的两种情况 (1) 调用queueInLoop的线程不是当前IO线程。(2)是当前IO线程并且正在调用pendingFunctor。
（第2点不太好理解，具体含义是:如果当前IO线程正在调用
doPendingFunctors( ),并且在该函数中调用了queueInloop,因此需要唤醒，以便让阻塞在poll系统调用的当前IO线程及时处理。）

**注意**
只有IO线程的事件回调(handleEvent)中调用queueInLoop才不需要唤醒。因为handleEvent处理完毕之后，接下来就是调用doPendingFunctors( )来处理任务。

### **Demo演示**

 **登录界面**

    
 ![image](https://user-images.githubusercontent.com/54012783/126977561-79159e09-10ee-4091-b7d5-e08cd302eb58.png)

 **注册界面**
    
    
 ![image](https://user-images.githubusercontent.com/54012783/126982004-3d1f7e10-e465-4441-a4ea-75dc6dc0b8d3.png)
    

  **计算界面**
    
    
  ![image](https://user-images.githubusercontent.com/54012783/126982260-5943f246-9dfd-4c6a-96e4-fd3bed64dba1.png)
    

  **计算结果界面**
    
    
  ![image](https://user-images.githubusercontent.com/54012783/126981874-a25c0e50-65de-4bfb-aebb-f17763e9c655.png)


  ### 压测结果
    
    系统配置：单核cpu，700M左右的可使用内存
    
    在关闭日志后，用Webbench对服务器进行压力测试，可实现上万的并发连接，测试结果如下：
    
  ![image](https://user-images.githubusercontent.com/54012783/127107035-14b90fce-e0ea-414c-958d-e40b8db818e3.png)

    
  
 ###  QPS:8166请求数/秒   吞吐量：8G/秒，每秒传输8G的数据量
