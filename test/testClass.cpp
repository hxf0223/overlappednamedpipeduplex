// testClass.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/thread/thread.hpp>
#include <boost/foreach.hpp>
#include<boost/format.hpp>
#include "scoped_handle.hpp"
#include "IocpNamedPipe.hpp"

#define ENABLE_TRACE
#include "trace.h"
#include "../OverlappedNamedPipeDuplex/IocpNamedPipe.hpp"

#define BOOST_DATE_TIME_SOURCE
#define BOOST_THREAD_NO_LIB

class testClass
{
public:
	explicit testClass( int id ) : m_id(id) {
		_trace("testClass construct %d\n", m_id);
	}

	testClass(testClass&& rhs) {
		_trace("rhs construct, rhs: %d\n", rhs.m_id);
		m_id = rhs.m_id;
		rhs.m_id = 0;
	}

	testClass& operator=(testClass&& rhs) {
		_trace("rhs operator=, rhs: %d\n", rhs.m_id);
		m_id = rhs.m_id;
		rhs.m_id = 0;
		return *this;
	}

	void set_value(int id) { m_id = id; }

	~testClass() {
		_trace("testClass destruct %d\n", m_id);
	}

private:
	static void test_thread(int* p) {
		for ( int i = 0; i < 10; i++ ) {
			_trace("test_thread run: %d, %d\n", p[0], p[1]);
			p[0] += 1; p[1] += 2;
			Sleep(100);
		}

		_trace("test_thread exit\n");
	}

private:
	int m_id;
};



int _tmain(int argc, _TCHAR* argv[])
{
	std::string temp = boost::str(boost::format("\n\n%s"
		"%1t ʮ���� = [%d]\n"
		"%1t ��ʽ����ʮ���� = [%5d]\n"
		"%1t ��ʽ��ʮ���ƣ�ǰ��'0' = [%05d]\n"
		"%1t ʮ������ = [%x]\n"
		"%1t �˽��� = [%o]\n"
		"%1t ���� = [%f]\n"
		"%1t ��ʽ���ĸ��� = [%3.3f]\n"
		"%1t ��ѧ���� = [%e]\n"
		) % "example :\n" % 15 % 15 % 15 % 15 % 15 % 15.01 % 15.01 % 15.01);

	//auto p = new testClass[10]{testClass(1), testClass(2), testClass(3), testClass(4), testClass(5), testClass(6), testClass(7), testClass(8), testClass(9), testClass(10)};
	boost::interprocess::overlapped::COverlappedNamedPipeDuplex pipecomm("named_pipe_comm_test", nullptr, nullptr);
	pipecomm.startServer();
	//pipecomm.startClient();

	while ( true ) {
		Sleep(10);
		pipecomm.startClient();
	}

	return 0;
}

