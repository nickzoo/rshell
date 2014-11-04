#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>

#define MICROSECONDS 1000000.0

class Timer {
public:
	void start();
	void stop();
	double wallclock_time();
	double user_time();
	double system_time();
	void print();

private:
	struct rusage r_start, r_stop;
	struct timeval t_start, t_stop;
};

void Timer::start() {
	gettimeofday(&t_start, 0);
	getrusage(RUSAGE_SELF, &r_start);
}

void Timer::stop() {
	gettimeofday(&t_stop, 0);
	getrusage(RUSAGE_SELF, &r_stop);
}

double Timer::wallclock_time() {
	return (t_stop.tv_sec - t_start.tv_sec) +
		(t_stop.tv_usec - t_start.tv_usec)/MICROSECONDS;
}

double Timer::user_time() {
	return (r_stop.ru_utime.tv_sec - r_start.ru_utime.tv_sec) +
		(r_stop.ru_utime.tv_usec - r_start.ru_utime.tv_usec)/MICROSECONDS;
}

double Timer::system_time() {
	return (r_stop.ru_stime.tv_sec - r_start.ru_stime.tv_sec) +
		(r_stop.ru_stime.tv_usec - r_start.ru_stime.tv_usec)/MICROSECONDS;
}

void Timer::print() {
	std::cout << "wallclock time: " << wallclock_time() << std::endl;
	std::cout << "user time: " << user_time() << std::endl;
	std::cout << "system time: " << system_time() << std::endl;
}
