#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif


#include "logging.h"
#include "ydx_timer.h"
#include "ydx_timerid.h"
#include "ydx_timer_set.h"
#include "epoller.h"

#include <boost/bind.hpp>
#include <sys/timerfd.h>

namespace ydx
{
int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
									TFD_NONBLOCK | TFD_CLOEXEC);
	if(timerfd < 0)
	{
		LOG_SYSFATAL << "Failed in timerfd_create";
	}
	return timerfd;
}

//返回一个时间距离目前时间的值 timespec表示
struct timespec howMuchTimeFromNow(Timestamp when)
{
	int64_t microseconds = when.get_micro_second()
	                     - Timestamp::now().get_micro_second();
	if (microseconds < 100)
	{
		microseconds = 100;
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(
	  microseconds / Timestamp::micro_seconds_per_sec);
	ts.tv_nsec = static_cast<long>(
	  (microseconds % Timestamp::micro_seconds_per_sec) * 1000);
	return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
	uint64_t howmany;
	ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
	//LOG_INFO << "TimerQueue::handleRead() " << howmany << " at " << now.to_string();
	if (n != sizeof howmany)
	{
		LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
	}
}

//重新设点timerfd的超时值，周期超时由程序timerset自己控制，不会写入itimerspec it_interval值
void resetTimerfd(int timerfd, Timestamp expiration)
{
	// wake up loop by timerfd_settime()
	struct itimerspec newValue;
	struct itimerspec oldValue;
	bzero(&newValue, sizeof newValue);
	bzero(&oldValue, sizeof oldValue);
	newValue.it_value = howMuchTimeFromNow(expiration);
	int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
	if (ret)
	{
		LOG_SYSERR << "timerfd_settime()";
	}
}

}

using namespace ydx;

TimerSet::TimerSet(EPollPoller *epoller)
	:epoller_(epoller),
	 timerfd_(createTimerfd()),
	 timerfd_channel_(epoller, timerfd_),
	 timer_container_()
{
	timerfd_channel_.setReadCallback(
				boost::bind(&TimerSet::handleRead, this));
	timerfd_channel_.enableReading();
}

TimerSet::~TimerSet()
{
	timerfd_channel_.disableAll();
	timerfd_channel_.remove();
	::close(timerfd_);
	for(TimerContainer::iterator it = timer_container_.begin();
			it != timer_container_.end(); ++it )
	{
		delete it->second;
	}
	
}

TimerId TimerSet::addTimer(const TimerCallback& cb,
                             Timestamp when,
                             double interval)
{
  Timer* timer = new Timer(cb, when, interval);
  epoller_->runInLoop(
      boost::bind(&TimerSet::addTimerInLoop, this, timer));
  return TimerId(timer, timer->sequence());
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
TimerId TimerSet::addTimer(TimerCallback&& cb,
                             Timestamp when,
                             double interval)
{
  Timer* timer = new Timer(std::move(cb), when, interval);
  epoller_->runInLoop(
      boost::bind(&TimerSet::addTimerInLoop, this, timer));
  return TimerId(timer, timer->sequence());
}
#endif

void TimerSet::cancel(TimerId timerId)
{
  epoller_->runInLoop(
      boost::bind(&TimerSet::cancelInLoop, this, timerId));
}

void TimerSet::addTimerInLoop(Timer* timer)
{
  //epoller_->assertInLoopThread();
	bool earliestChanged = insert(timer);

	if (earliestChanged)
	{
		resetTimerfd(timerfd_, timer->expiration());
	}
}


void TimerSet::cancelInLoop(TimerId timerId)
{
//	epoller_->assertInLoopThread();

	//ActiveTimer timer(timerId.timer_, timerId.sequence_);

	timer_container_.erase(Entry(timerId.timer_->expiration(), timerId.timer_));
	
	delete timerId.timer_; // FIXME: no delete please
    
 
}

void TimerSet::handleRead()
{
//  epoller_->assertInLoopThread();
  Timestamp now(Timestamp::now());
  readTimerfd(timerfd_, now);

  std::vector<Entry> expired = getExpired(now);


  // safe to callback outside critical section
  for (std::vector<Entry>::iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    it->second->run();
  }

  reset(expired, now);
}

std::vector<TimerSet::Entry> TimerSet::getExpired(Timestamp now)
{
  
  std::vector<Entry> expired;
  Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  TimerContainer::iterator end = timer_container_.lower_bound(sentry);
  assert(end == timer_container_.end() || now < end->first);
  std::copy(timer_container_.begin(), end, back_inserter(expired));
  timer_container_.erase(timer_container_.begin(), end);

  return expired;
}
//对已经超时的结点，如果不是周期性调用，则删除结点，如果周期性调用，则重新加入到时间容器
void TimerSet::reset(const std::vector<Entry>& expired, Timestamp now)
{
  Timestamp nextExpire;

	for (std::vector<Entry>::const_iterator it = expired.begin();
	  it != expired.end(); ++it)
	{

		if (it->second->repeat())
		{
			it->second->restart(now);
			insert(it->second);
		}
		else
		{
		  // FIXME move to a free list
		  delete it->second; // FIXME: no delete please
		}
	}

	if (!timer_container_.empty())
	{
		nextExpire = timer_container_.begin()->second->expiration();
	}

	if (nextExpire.valid())
	{
		resetTimerfd(timerfd_, nextExpire);
	}
}

bool TimerSet::insert(Timer* timer)
{
//  epoller_->assertInLoopThread();
  bool earliestChanged = false;
  Timestamp when = timer->expiration();
  TimerContainer::iterator it = timer_container_.begin();
  if (it == timer_container_.end() || when < it->first)
  {
    earliestChanged = true;
  }
  {
    std::pair<TimerContainer::iterator, bool> result
      = timer_container_.insert(Entry(when, timer));
    assert(result.second); (void)result;
  }
  return earliestChanged;
}


