# ydx_timer_util
定时器方案，采用epoll的异步驱动和采用时间轮推动两种方案。测试在20万个定时器同时存在(以50us的速率仍然持续创建)的情况下内存为20M，cpu占用2%左右
1.timer_set为timerfd方式，用一个时间句柄控制所有定时器节点。
2.timer_wheel为时间轮推动方式。
