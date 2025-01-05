#pragma once
#include <iostream>
#include <chrono>

class Timer {
public:
	Timer() {
		last_time = std::chrono::steady_clock::now();
	}

	std::chrono::steady_clock::time_point CurrentTime() {
		auto now = std::chrono::steady_clock::now();
		return now;
	}

	uint64_t CalcDuration(std::chrono::steady_clock::time_point point_in_time)	{
		auto now = std::chrono::steady_clock::now();
		uint64_t Duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - point_in_time).count();
		return Duration;		
	}

	uint64_t GetDeltaTimeNS() {
	 	auto now	  = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_time);
	 	last_time = now;
	 	return duration.count();  // Return delta time in milliseconds as integer
	}

	bool TimePassed(uint64_t TimePassed, bool bPrint = false) {
		auto now = std::chrono::steady_clock::now();
		uint64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count();

		if (duration >= TimePassed)
		{
			if (bPrint) {
				std::cout << "TimePassed =" << duration << "\n";
			}			
			last_time = now;
			return true; 
		}		
		return false;
	}

private:
	std::chrono::steady_clock::time_point last_time;
};