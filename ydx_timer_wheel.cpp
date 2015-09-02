#include "ydx_timer_wheel.h"
#include "logging.h"
using namespace ydx;

bool TimerWheel::addNode(Timestamp time, TimerCallback cb, WeakNodePtr &weak_ptr)
{
	NodePtr ptr_node(new TimerNode(time, cb));
	if(NULL == ptr_node) return false;
	//计算应该向前走时间轮的几个格子
	int64_t diff_time = timeDifferenceMicro(time, time_);
	//LOG_INFO<<"diff:"<<diff_time;
	if( diff_time < 0 ) return false;	

	//刷新时间轮的最新时间
	if(diff_time > 0)
	{
		time_.set_micro_second(time);
	}
	for(int64_t i = 0; i < diff_time; ++i)
	{
		//插入一个hashset，队列首元素自动被弹出析构
		node_circle_buf_.push_back(Bucket());
	}
	node_circle_buf_.back().push_back(ptr_node);
	
	weak_ptr = ptr_node;
	return true;
}

void TimerWheel::refreshNode(WeakNodePtr weak_ptr_node)
{
	NodePtr ptr_node(weak_ptr_node.lock());
	if(ptr_node)
	{
		node_circle_buf_.back().push_back(ptr_node);
	}
	else
	{
		LOG_INFO << "weak_ptr_node is null";
	}
}



