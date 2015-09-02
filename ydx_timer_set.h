/*
timer_set目的在于实现一个用epoll驱动的异步事件回调定时器
1.程序可以和业务处理过程共同位于epoll_wait循环中，此时超时处理不存在临界区。
2.程序可以和业务处理过程位于不同的线程，此时超时处理需要判断timer_node的结点是否需要同步处理。
*/

#ifndef __YDX_TIMER_SET_H__
#define __YDX_TIMER_SET_H__

#include <set>
#include <vector>
#include <boost/noncopyable.hpp>
#include "callback_types.h"
#include "timestamp.h"
#include "channel.h"


namespace ydx
{

class EPollPoller;
class Timer;
class TimerId;

class TimerSet : boost::noncopyable
{
public:
	TimerSet(EPollPoller *epoller);
	~TimerSet();
 public:

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.
  
  //addtimer为外部调用接口
  
  //调用层次addTimer->addTimerInLoop->insert
  TimerId addTimer(const TimerCallback& cb,
                   Timestamp when,
                   double interval);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  TimerId addTimer(TimerCallback&& cb,
                   Timestamp when,
                   double interval);
#endif

  void cancel(TimerId timerId);

private:
	//存放在定时器结点容器Set中的元素，采用pair保证当Timestamp相同时，Timer的地址不同
	//因此可以存放相同的时间值元素
	typedef std::pair<Timestamp, Timer*> Entry;
	//定时器容器存放所有计时的结点(std::pair)
	typedef std::set<Entry> TimerContainer;
	//
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;
	//addTimerInloop由addtimer的外部接口调用
	void addTimerInLoop(Timer* timer);
	void cancelInLoop(TimerId timerId);
	// called when timerfd alarms
	void handleRead();
	//getExpired获取所有的超时结点存放到vector
	std::vector<Entry> getExpired(Timestamp now);
	//对周期调用的定时器，做重新启动
	void reset(const std::vector<Entry>& expired, Timestamp now);
	//内部做定时器插入
	bool insert(Timer* timer);
private:
	EPollPoller *epoller_;
	const int timerfd_;//timerfd_create创建
	Channel   timerfd_channel_; //放入epoller循环中做监测
	TimerContainer timer_container_;//时间结点容器

	//ActiveTimerSet active_timers_;
};

}

#endif