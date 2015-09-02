/*
timer_wheel目的在于实现一个用传入时间轮的一个时间，比如码流时间，推动的定时器。
timer_wheel总是应该与业务逻辑出于同一个线程，在收到新时间的时候做一次整个时间轮的的结点超时判断。
timer_wheel使用boost::circle_buffer和shared_ptr
*/

#ifndef __YDX_TIMER_WHEEL_H__
#define __YDX_TIMER_WHEEL_H__
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/unordered_set.hpp>
#include <boost/version.hpp>
#include <list>
#include "timestamp.h"
#include "callback_types.h"
#if BOOST_VERSION < 104700
namespace boost
{
template <typename T>
inline size_t hash_value(const boost::shared_ptr<T>& x)
{
	return boost::hash_value(x.get());
}
}
#endif

namespace ydx
{
struct TimerNode
{
	TimerNode(Timestamp time, TimerCallback cb)
		:time_(time),
		 cb_(cb)
	{

	}
	~TimerNode()
	{
		cb_();
	}
	
	Timestamp time_;
	TimerCallback cb_;
};


class TimerWheel : boost::noncopyable
{
public:
	
	TimerWheel(int bucket_num = 10000)
		:node_circle_buf_(bucket_num),
		bucket_num_(bucket_num),
		time_(Timestamp::now())
	{
		node_circle_buf_.resize(bucket_num);
	}


	
	typedef boost::shared_ptr<TimerNode> NodePtr;
	typedef boost::weak_ptr<TimerNode> WeakNodePtr;
	//typedef boost::unordered_set<NodePtr> Bucket;
	typedef std::list<NodePtr> Bucket;
	typedef boost::circular_buffer<Bucket> CircleBuff;
	
	bool addNode(Timestamp time, TimerCallback cb, WeakNodePtr &weak_ptr);
	void refreshNode(WeakNodePtr weak_ptr_node);

	CircleBuff node_circle_buf_;
	int bucket_num_;
	Timestamp time_; //记录最近一个放入时间轮的时间
};

}


#endif
