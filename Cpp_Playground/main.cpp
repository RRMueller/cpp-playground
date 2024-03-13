#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <random>

typedef struct
{
	uint8_t byte;
	uint8_t bit;
	uint8_t len;
} SPN_Config;

long long epoch();
long long epochMillis();
uint64_t hours();
uint64_t minutes();
uint64_t seconds();
uint64_t millis();
uint64_t micros();
uint64_t nanos();
bool timerMillis(uint64_t* prevTime, uint64_t timeout, bool resetPrevTime, uint64_t current_time, bool useFakeMillis);
double scale(double input, double minIn, double maxIn, double minOut, double maxOut, bool clipOutput);

int64_t random(void);
int64_t random(int64_t max);
int64_t random(int64_t min, int64_t max);
bool random(bool* trigger, int64_t* output, bool noWait);
bool testRandom(bool noWait, uint8_t* count);
int getBoolFromCanTelegram(uint8_t telegram[], uint8_t sizeOfTelegram, bool* output, SPN_Config spnConfig);
int getIntFromCanTelegram(uint8_t telegram[], uint8_t sizeOfTelegram, int* output, SPN_Config spnConfig);

auto startTime = std::chrono::steady_clock::now(); //steady clock is great for timers, not great for epoch




int main()
{
	for (;;)
	{
		static uint64_t prevPrintTime = 0;
		uint64_t printTimeout = 2000;
		uint8_t count = 0;

		int canDiagTxID = 0x18FF0000;  //  ID for diagnostics messages we send to CAN bus
		int canDiagTxID_1 = canDiagTxID | (1 << 8);  //  0x18FF0100

		//uint8_t sizeOfTelegram = 8;
		//static uint8_t telegram[] = { 0b00000001,0b00000100,0b00010000,0b01000000,5,6,7,8};
		//bool trueFalse = false;
		//SPN_Config testSPN = { 2, 4, 11 };
		//int output = 0;
		//int byte = 2;
		//int bit = 4;
		//int length = 11;
		char escape = 1;
		char hold = 1;

		if (timerMillis(&prevPrintTime, printTimeout, true, 0, false))
		{
			//getBoolFromCanTelegram(telegram, sizeOfTelegram, &trueFalse, byte, bit, length);
			//getIntFromCanTelegram(telegram, sizeOfTelegram, &output, testSPN);
			//printf("trueFalse: %s\n", trueFalse ? "true" : "False");

			escape = 0;
			hold = 0;
			printf("escape: %d\n", escape);
			printf("!escape: %d\n", !escape);
			printf("hold: %d\n", hold);
			printf("!hold: %d\n", !hold);
			printf("escape & hold: %d\n", escape & hold);
			printf("escape & !hold: %d\n", escape & !hold);
			printf("escape && hold: %d\n", escape && hold);
			printf("escape && !hold: %d\n", escape && !hold);
			printf("~~~~~~~~~~~\n");
			escape = 0;
			hold = 1;
			printf("escape: %d\n", escape);
			printf("!escape: %d\n", !escape);
			printf("hold: %d\n", hold);
			printf("!hold: %d\n", !hold);
			printf("escape & hold: %d\n", escape & hold);
			printf("escape & !hold: %d\n", escape & !hold);
			printf("escape && hold: %d\n", escape && hold);
			printf("escape && !hold: %d\n", escape && !hold);
			printf("~~~~~~~~~~~\n");
			escape = 1;
			hold = 0;
			printf("escape: %d\n", escape);
			printf("!escape: %d\n", !escape);
			printf("hold: %d\n", hold);
			printf("!hold: %d\n", !hold);
			printf("escape & hold: %d\n", escape & hold);
			printf("escape & !hold: %d\n", escape & !hold);
			printf("escape && hold: %d\n", escape && hold);
			printf("escape && !hold: %d\n", escape && !hold);
			printf("~~~~~~~~~~~\n");
			escape = 1;
			hold = 1;
			printf("escape: %d\n", escape);
			printf("!escape: %d\n", !escape);
			printf("hold: %d\n", hold);
			printf("!hold: %d\n", !hold);
			printf("escape & hold: %d\n", escape & hold);
			printf("escape & !hold: %d\n", escape & !hold);
			printf("escape && hold: %d\n", escape && hold);
			printf("escape && !hold: %d\n", escape && !hold);
			printf("~~~~~~~~~~~\n");
			escape = 2;
			hold = 0;
			printf("escape: %d\n", escape);
			printf("!escape: %d\n", !escape);
			printf("hold: %d\n", hold);
			printf("!hold: %d\n", !hold);
			printf("escape & hold: %d\n", escape & hold);
			printf("escape & !hold: %d\n", escape & !hold);
			printf("escape && hold: %d\n", escape && hold);
			printf("escape && !hold: %d\n", escape && !hold);
			printf("~~~~~~~~~~~\n");
			escape = 0;
			hold = 2;
			printf("escape: %d\n", escape);
			printf("!escape: %d\n", !escape);
			printf("hold: %d\n", hold);
			printf("!hold: %d\n", !hold);
			printf("escape & hold: %d\n", escape & hold);
			printf("escape & !hold: %d\n", escape & !hold);
			printf("escape && hold: %d\n", escape && hold);
			printf("escape && !hold: %d\n", escape && !hold);
			printf("~~~~~~~~~~~\n");
			return(0);
			//if (telegram[testSPN.byte] == 0)
			//{
				//telegram[byte] = 1;
			//}
			//else
			//{
				//telegram[byte] = 0;
			//}
		}
	}
	return 0;
}


int getBoolFromCanTelegram(uint8_t telegram[], uint8_t sizeOfTelegram, bool* output, SPN_Config spnConfig)
{
	if ((8 * spnConfig.byte + spnConfig.bit + spnConfig.len) > (sizeOfTelegram * 8)) // Check if we are asking for something outside of telegram's allocation
		return -1;	// Return -1 if we overrun the array?

	int mask = 0;
	int i = 0;
	for (i; i < spnConfig.len; i++)
	{
		mask |= (1 << i);
	}
	int val = (telegram[spnConfig.byte] >> spnConfig.bit) & mask;
	if (val)
		*output = true;
	else
		*output = false;
	return 0;
}

int getIntFromCanTelegram(uint8_t telegram[], uint8_t sizeOfTelegram, int* output, SPN_Config spnConfig)
{
	if ((8 * spnConfig.byte + spnConfig.bit + spnConfig.len) > (sizeOfTelegram * 8)) // Check if we are asking for something outside of telegram's allocation
		return -1;	// Return -1 if we overrun the array?

	int mask = 0;
	int i = 0;
	for (i; i < spnConfig.len; i++)
	{
		mask |= (1 << i);
	}
	int val = 0;
	uint8_t numBytes = 1 + (spnConfig.bit + spnConfig.len) / 8;
	i = 0;
	for (i; i < numBytes; i++)
	{
		val |= telegram[spnConfig.byte + i] << (8 * i);
	}
	*output = (val >> spnConfig.bit) & mask;
	return 0;
}


bool testRandom(bool noWait, uint8_t* count)
{
	bool success = false;
	do
	{
		*count += 1;
		if (random(5) == 5)
		{
			success = true;
		}
	} while (noWait && !success);
	return success;
}


/**
 * @brief Generates a 64-bit random number between `0` and `0x7FFFFFFFFFFFFFFF`. This is
 *			also slower compared to `random(*trigger, *output)` since it waits for the RNG to
 *			successfully generate.
 *
 * @return Returns random number between `0` and `0x7FFFFFFFFFFFFFFF`.
 */
int64_t random(void)
{
	return random(0x7FFFFFFFFFFFFFFF);
}

/**
 * @brief Generates a 64-bit random number between `0` and `max`. This is also slower
 *			compared to `random(*trigger, *output)` since it waits for the RNG to
 *			successfully generate.
 *
 * @param max Maximum bound for output
 * @return Returns random number between `0` and `max`.
 */
int64_t random(int64_t max)
{
	int64_t output;
	bool trigger = true;
	if (max == 0)
	{
		return 0;
	}
	if (random(&trigger, &output, true))
	{
		if (max == -1)	//	Edge conditions...
		{// Do nothing
		}
		else if (max != 0x7FFFFFFFFFFFFFFF)	//	Be inclusive on max value
		{
			max++;
		}
		return output % (max);
	}
	return 0;
}

/**
 * @brief Generates a 64-bit random number between `min` and `max`. NOTE: difference between 
			`max` and `min` must be smaller than `0x7FFFFFFFFFFFFFFF`!! This is also slower
			compared to `random(*trigger, *output)` since it waits for the RNG to 
			successfully generate. 
 *
 * @param min Minimum bound for output
 * @param max Maximum bound for output
 * @return Returns random number between `min` and `max`.
 */
int64_t random(int64_t min, int64_t max)
{
	int64_t minVal = (min < max) ? min : max;	//	Find min/max values incase someone gave us parameters in the wrong order
	int64_t maxVal = (min > max) ? min : max;
	printf("minVal = %lld, maxVal = %lld\n", minVal, maxVal);
	int64_t diff = maxVal - minVal;
	return random(diff) + minVal;
}

/**
 * @brief Generates a 64-bit random number
 *
 * @param trigger Start RNG generation
 * @param *output if RNG generation was a success, the generated random number
 * @param noWait if TRUE, wait until generation is complete before returning, otherwise return even if generation is not complete
 * @return If RNG is sucessful, returns true, otherwise false
 */
bool random(bool* trigger, int64_t* output, bool noWait)
{
#define RND_SIZE_DU16 8 // global data definitions
	bool success = false;
	uint8_t rnd_au8[RND_SIZE_DU16]; // array for storage of random
	// static bool getRandom_l = FALSE;
	uint16_t result_u16;
	if (*trigger != false) // check for request to generate a new random
	{
		*output = ((int64_t)rand() << 32) | rand();
		*trigger = false;
		success = true;
	}
	return success;
}


double scale(double input, double minIn, double maxIn, double minOut, double maxOut, bool clipOutput)
{
	double slope = ((maxOut - minOut) / (maxIn - minIn));
	double intercept = (minOut - (minIn * slope));
	double output = ((slope * input) + intercept);
	if (clipOutput) //  DON'T ALLOW OUTPUT OUTSIDE RANGE
	{
		double minVal = (minOut < maxOut) ? minOut : maxOut;	//	FIND MIN/MAX VALUES - INCASE WE ARE INVERSELY SCALING
		double maxVal = (minOut > maxOut) ? minOut : maxOut;
		if (output > maxVal)
			output = maxVal;
		if (output < minVal)
			output = minVal;
	}
	return output;
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