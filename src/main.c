#include <Arduino.h>

#define second 1000000 // 1 sec in microseconds
#define pwm_freq 200   // wanted "pwm" frequency
#define fade_freq 50   // time to change "pwm" vals per second
#define pwm_steps 50   // amount of "pwm" brightness steps

// Shift register pins
uint8_t SER = PD2;	// input to shift reg 1
uint8_t SCLK = PC6; // shift register clock
uint8_t RCLK = PC7; // storage register clock (output)

uint_fast16_t pwm_time = second / pwm_freq;		 // pwm loop time eg. freq
uint_fast16_t pwm_step;							 // single brigtness value step
unsigned long fade_time = second / fade_freq;	 // time between brightness steps
uint_fast16_t pwm_ontimes[16] = {0};			 // on time within a pwm loop for each led
uint_fast16_t increments[16];					 // increments (+- pwm_step) per led applied each fade cycle
unsigned long main_start, fade_start, pwm_start; // micros() storage variables
uint16_t diff;									 // used in pwm calc
byte random_led;
uint16_t shift_data;

// shift 16 bits to shift register output
void shift16(uint16_t data)
{
	for (size_t i = 0; i < 16; i++)
	{
		digitalWrite(SER, (data >> i) & 1);
		digitalWrite(SCLK, LOW);
		digitalWrite(SCLK, HIGH);
	}
	digitalWrite(RCLK, LOW);
	digitalWrite(RCLK, HIGH);
}

// Output single "PWM" with duty cycle set by "pwm_ontimes"
void pwm_loop()
{
	pwm_start = micros();
	shift_data = 0xFFFF;
	diff = 0;
	while (diff < pwm_time)
	{
		for (size_t i = 0; i < 16; i++)
		{
			if (diff >= pwm_ontimes[i])
				shift_data &= ~(1 << i);
		}
		shift16(shift_data);
		diff = micros() - pwm_start;
	}
}

void setup()
{
	pinMode(SER, OUTPUT);
	pinMode(SCLK, OUTPUT);
	pinMode(RCLK, OUTPUT);
	randomSeed(analogRead(PC3));
	pwm_step = pwm_time / pwm_steps;
	for (size_t i = 0; i < 16; i++)
		increments[i] = pwm_step;
}

void loop()
{
	main_start = micros();
	while (micros() - main_start < 300000) // interval between random selections of led to turn on
	{
		fade_start = micros();
		while (micros() - fade_start < fade_time)
			pwm_loop();
		for (size_t i = 0; i < 16; i++) // step ontimes to alter "pwm"
		{
			if (pwm_ontimes[i] > 0)
				pwm_ontimes[i] += increments[i];
			if (pwm_ontimes[i] >= pwm_time)
				increments[i] = -1 * pwm_step;
		}
	}
	random_led = random(16);
	if (pwm_ontimes[random_led] == 0) // only turn on if led off
	{
		increments[random_led] = pwm_step;
		pwm_ontimes[random_led] = pwm_step;
	}
}
