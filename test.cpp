#include <iostream>
#include <cassert>
#include <cstring>
#include "Message.h"

void test1()
{
	Message msg(Message::SetText);
	assert(msg.size() == 0);
	msg.setText("Abc 123");
	msg.prepare();
	assert(msg.size() == 1 + strlen("Abc 123"));
	assert(strcmp("Abc 123", msg.text()) == 0);
	assert(strcmp("Avc 123", msg.text()) != 0);

	std::cout << "Test 1 OK" << std::endl;
}

void test2()
{

}

void test3()
{

}

void test4()
{

}

int main()
{
	test1();
	test2();
	test3();
	test4();
	return 0;
}