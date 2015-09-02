#ifndef __YDX_TIMER_H__
#define __YDX_TIMER_H__

#include <boost/noncopyable.hpp>
#include "timestamp.h"
#include "callback_types.h"
#include "ydx_atomic.h"

namespace ydx
{

class Timer : boost::noncopyable
{
public:
	
	Timer(const TimerCallback &cb, Timestamp when, double interval)
	: timer_func_(cb),
	  expiration_(when),
	  interval_(interval),
	  repeat_(interval > 0.0),
	  sequence_(timer_sequence_creater.incrementAndGet())
	{}

	void run() const
	{
		timer_func_();
	}

	Timestamp expiration() const { return expiration_; }
	bool repeat() const {return repeat_; }
	int64_t sequence() const {return sequence_; }
	void restart(Timestamp now);
	static int64_t index_create() {return timer_sequence_creater.get();}
	
private:
	const TimerCallback timer_func_;
	Timestamp expiration_;
	const double interval_;
	const bool repeat_;
	const int64_t sequence_;
	
	
	static AtomicInt64 timer_sequence_creater;
};


	
}

#endif