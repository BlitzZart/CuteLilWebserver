// NDS16_webserver.cpp : Defines the entry point for the console application.

#include <thread>
#include <iostream>
#include <string>
#include <mutex>
#include <chrono>

using namespace std;

mutex mtx;

void test_thread(string name, int rate) {
	for (int i = 0; i < 10; i++) {
		mtx.lock();
		cout << "Thread " << name << endl;
		mtx.unlock();
		this_thread::sleep_for(chrono::milliseconds(rate));
	}
}
void threadFun()  {
	thread t1(test_thread, "lorenz", 111);
	thread t2(test_thread, "dani", 321);
	t1.join();
	t2.join();
}

int NOmain()
{
	string s;

	cout << "start" << endl;
	threadFun();


	cin >> s;

	return 0;
}