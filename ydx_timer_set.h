/*
timer_setĿ������ʵ��һ����epoll�������첽�¼��ص���ʱ��
1.������Ժ�ҵ������̹�ͬλ��epoll_waitѭ���У���ʱ��ʱ���������ٽ�����
2.������Ժ�ҵ�������λ�ڲ�ͬ���̣߳���ʱ��ʱ������Ҫ�ж�timer_node�Ľ���Ƿ���Ҫͬ������
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
  
  //addtimerΪ�ⲿ���ýӿ�
  
  //���ò��addTimer->addTimerInLoop->insert
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
	//����ڶ�ʱ���������Set�е�Ԫ�أ�����pair��֤��Timestamp��ͬʱ��Timer�ĵ�ַ��ͬ
	//��˿��Դ����ͬ��ʱ��ֵԪ��
	typedef std::pair<Timestamp, Timer*> Entry;
	//��ʱ������������м�ʱ�Ľ��(std::pair)
	typedef std::set<Entry> TimerContainer;
	//
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;
	//addTimerInloop��addtimer���ⲿ�ӿڵ���
	void addTimerInLoop(Timer* timer);
	void cancelInLoop(TimerId timerId);
	// called when timerfd alarms
	void handleRead();
	//getExpired��ȡ���еĳ�ʱ����ŵ�vector
	std::vector<Entry> getExpired(Timestamp now);
	//�����ڵ��õĶ�ʱ��������������
	void reset(const std::vector<Entry>& expired, Timestamp now);
	//�ڲ�����ʱ������
	bool insert(Timer* timer);
private:
	EPollPoller *epoller_;
	const int timerfd_;//timerfd_create����
	Channel   timerfd_channel_; //����epollerѭ���������
	TimerContainer timer_container_;//ʱ��������

	//ActiveTimerSet active_timers_;
};

}

#endif