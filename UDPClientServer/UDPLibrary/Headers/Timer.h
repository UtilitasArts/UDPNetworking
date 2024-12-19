#pragma once
#include <iostream>
#include <chrono>
#include <thread>

class Timer {
public:
	Timer() {
		last_time = std::chrono::steady_clock::now();
	}
	// Get delta time in milliseconds as an integer
	// 	int getDeltaTimeMS() {
	// 		auto now = std::chrono::steady_clock::now();
	// 		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count();
	// 		last_time = now;
	// 		return static_cast<int>(duration);  // Return delta time in milliseconds as integer
	// 	}

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