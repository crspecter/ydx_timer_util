#ifndef __YDX_TIMERID_H__
#define __YDX_TIMERID_H__

namespace ydx
{
class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId 
{
 public:
  TimerId()
    : timer_(NULL),
      sequence_(0)
  {
  }

  TimerId(Timer* timer, int64_t seq)
    : timer_(timer),
      sequence_(seq)
  {
  }

  // default copy-ctor, dtor and assignment are okay

  friend class TimerSet;

 private:
  Timer* timer_;
  int64_t sequence_;
};

}


#endif
