#include "ydx_timer_wheel.h"
#include "logging.h"
using namespace ydx;

bool TimerWheel::addNode(Timestamp time, TimerCallback cb, WeakNodePtr &weak_ptr)
{
	NodePtr ptr_node(new TimerNode(time, cb));
	if(NULL == ptr_node) return false;
	//����Ӧ����ǰ��ʱ���ֵļ�������
	int64_t diff_time = timeDifferenceMicro(time, time_);
	//LOG_INFO<<"diff:"<<diff_time;
	if( diff_time < 0 ) return false;	

	//ˢ��ʱ���ֵ�����ʱ��
	if(diff_time > 0)
	{
		time_.set_micro_second(time);
	}
	for(int64_t i = 0; i < diff_time; ++i)
	{
		//����һ��hashset��������Ԫ���Զ�����������
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



