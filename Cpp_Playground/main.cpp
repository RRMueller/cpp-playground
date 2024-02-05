#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>

long long epoch();
long long epochMillis();
uint64_t hours();
uint64_t minutes();
uint64_t seconds();
uint64_t millis();
uint64_t micros();
uint64_t nanos();
bool timerMillis(uint64_t* prevTime, uint64_t timeout, bool resetPrevTime, uint64_t current_time, bool useFakeMillis);

auto startTime = std::chrono::steady_clock::now(); //steady clock is great for timers, not great for epoch

int main()
{
	for (;;)
	{
		static uint64_t prevPrintTime = 0;
		while (!timerMillis(&prevPrintTime, 1000, true, 0, false))
		{
		}
		printf("Hello, World!\n");
	}
	return 0;
}


//---------------------------------------------------------------------------------------------------------
// timerMillis() - RETURNS WHETHER THE DURATION OF TIME SINCE prevTime IS GREATER THAN OR EQUAL TO timeout
// parameters:  prevTime - TIME START
//              timeout - DURATION OF TIME SINCE prevTime TO TRIGGER
//              resetPrevTime - RESET prevTime ONCE DURATION IS REACHED
//              current_time - USE WHEN useFakeMillis = true (1)
//              useFakeMillis - USE current_time INSTEAD OF GETTING millis().  USEFUL WHEN CALLING
//                              FUNCTION OFTEN AND RUNTIME SPEED PRIORITIZES TIMER ACCURACY 
//
// returns:     true (1) = IF current_time (OR millis()) IS GREATER THAN OR EQUAL TO prevTime + timeout
//              false (0) = IF NOT true
//---------------------------------------------------------------------------------------------------------
bool timerMillis(uint64_t* prevTime, uint64_t timeout, bool resetPrevTime, uint64_t current_time, bool useFakeMillis)
{
	if (!useFakeMillis)	   //  IF WE DON'T SAY TO USE OUR GIVEN MILLIS, LET'S CHECK MILLIS OURSELVES!
	{
		current_time = millis();
	}
	if ((uint64_t)(current_time - *prevTime) >= timeout)	  // typecast to create a massive positive number when current_time rolls over to 0
	{
		if (resetPrevTime)
		{
			*prevTime = millis();
		}
		return 1;
	}
	return 0;
}

long long epoch()
{
	long long milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return milliseconds_since_epoch;
}

long long epochMillis()
{
	//returning epoch from 1970
	long long milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return milliseconds_since_epoch;
}

//return a 64 bit millis() since the program started. This makes the output very similar to (not) Arduino's hours() function
uint64_t hours()
{
	auto now = std::chrono::steady_clock::now();
	auto now_ms = std::chrono::duration_cast<std::chrono::hours>(now - startTime).count();
	uint64_t ms = (uint64_t)now_ms;
	return ms;
}

//return a 64 bit millis() since the program started. This makes the output very similar to (not) Arduino's minutes() function
uint64_t minutes()
{
	auto now = std::chrono::steady_clock::now();
	auto now_ms = std::chrono::duration_cast<std::chrono::minutes>(now - startTime).count();
	uint64_t ms = (uint64_t)now_ms;
	return ms;
}

//return a 64 bit millis() since the program started. This makes the output very similar to (not) Arduino's seconds() function
uint64_t seconds()
{
	auto now = std::chrono::steady_clock::now();
	auto now_ms = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
	uint64_t ms = (uint64_t)now_ms;
	return ms;
}

//return a 64 bit millis() since the program started. This makes the output very similar to Arduino's millis() function
uint64_t millis()
{
	auto now = std::chrono::steady_clock::now();
	auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
	uint64_t ms = (uint64_t)now_ms;
	return ms;
}

//return a 64 bit micros() since the program started. This makes the output very similar to Arduino's micros() function
uint64_t micros()
{
	auto now = std::chrono::steady_clock::now();
	auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
	uint64_t us = (uint64_t)now_us;
	return us;
}

//return a 64 bit nanos() since the program started. This makes the output very similar to (not) Arduino's nanos() function
uint64_t nanos()
{
	auto now = std::chrono::steady_clock::now();
	auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - startTime).count();
	uint64_t ns = (uint64_t)now_ns;
	return  ns;
}