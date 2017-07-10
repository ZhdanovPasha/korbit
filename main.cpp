#include <stdio.h>
#include <iostream>
#include <chrono>
#include "ThreadPool.h"

int main(int argc, char** argv) {

	int nthreads = std::thread::hardware_concurrency();
	ThreadPool myPool(nthreads);
	// use this for this_thread::sleep_for(s);
	using namespace std::chrono_literals;
	auto start = std::chrono::high_resolution_clock::now();
	// test
	for (int i = 0; i < 10; i++) {
		myPool.addTask([] {std::this_thread::sleep_for(1s); });
	}
	//myPool.joinAll();
	myPool.waitAll();
	for (int i = 0; i < 10; i++) {
		myPool.addTask([] {std::this_thread::sleep_for(1s); });
	}
	//myPool.joinAll();
	//myPool.waitAll();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	printf ("Programm successfully finished. Time: %.5lf ms\n", elapsed.count()) ;
	system("pause");
	return 0;
}
