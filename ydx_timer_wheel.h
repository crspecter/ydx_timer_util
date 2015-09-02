/*
timer_wheelĿ������ʵ��һ���ô���ʱ���ֵ�һ��ʱ�䣬��������ʱ�䣬�ƶ��Ķ�ʱ����
timer_wheel����Ӧ����ҵ���߼�����ͬһ���̣߳����յ���ʱ���ʱ����һ������ʱ���ֵĵĽ�㳬ʱ�жϡ�
timer_wheelʹ��boost::circle_buffer��shared_ptr
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
	Timestamp time_; //��¼���һ������ʱ���ֵ�ʱ��
};

}


#endif
