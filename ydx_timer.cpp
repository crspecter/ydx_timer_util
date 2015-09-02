#include  "ydx_timer.h"

using namespace ydx;


AtomicInt64 Timer::timer_sequence_creater;

void Timer::restart(Timestamp now)
{
	if(repeat_)
	{
		expiration_ = addTime(now, interval_);
	}
	else
	{
		expiration_ = Timestamp::invalid();
	}
}