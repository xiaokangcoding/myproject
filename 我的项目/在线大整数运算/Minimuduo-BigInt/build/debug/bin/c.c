main(): pid = 3288, flag = 0
20210512 14:34:53.039757Z  3288 TRACE updateChannel fd = 4 events = 3 - EPollPoller.cc:104
20210512 14:34:53.039804Z  3288 TRACE EventLoop EventLoop created 0x7FFD4DAFC420 in thread 3288 - EventLoop.cc:62
20210512 14:34:53.039806Z  3288 TRACE updateChannel fd = 5 events = 3 - EPollPoller.cc:104
20210512 14:34:53.039816Z  3288 TRACE loop EventLoop 0x7FFD4DAFC420 start looping - EventLoop.cc:94
20210512 14:34:55.040639Z  3288 TRACE poll 1 events happended - EPollPoller.cc:65
20210512 14:34:55.040756Z  3288 TRACE printActiveChannels {4: IN }  - EventLoop.cc:257
20210512 14:34:55.040774Z  3288 TRACE readTimerfd TimerQueue::handleRead() 1 at 1620830095.040761 - TimerQueue.cc:62
run1(): pid = 3288, flag = 1
run2(): pid = 3288, flag = 1
run3(): pid = 3288, flag = 2
20210512 14:34:58.041420Z  3288 TRACE poll 1 events happended - EPollPoller.cc:65
20210512 14:34:58.041456Z  3288 TRACE printActiveChannels {4: IN }  - EventLoop.cc:257
20210512 14:34:58.041467Z  3288 TRACE readTimerfd TimerQueue::handleRead() 1 at 1620830098.041460 - TimerQueue.cc:62
run4(): pid = 3288, flag = 3
20210512 14:34:58.041478Z  3288 TRACE loop EventLoop 0x7FFD4DAFC420 stop looping - EventLoop.cc:119
main(): pid = 3288, flag = 3
