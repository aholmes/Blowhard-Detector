#include <Time.h>

#include <7segment.h>
#include <font5x7.h>
#include <font8x16.h>
#include <fontlargenumber.h>
#include <MicroView.h>
#include <space01.h>
#include <space02.h>
#include <space03.h>

#include <math.h>

/**
 * The maximum number of values that can be read from an analog input
 */
const int ADCMAX = 1023;

/**
 * The number of horizontal pixels on the MicroView display
 */
const int XMAX = 64;

/**
 * The number of vertical pixels on the MicroView display
 */
const int YMAX = 48;

/**
 * The desired display width of each drawn line
 */
const int BARWIDTH = 1;

/**
 * An optional delay between each loop execution. If 0, there will not be a delay.
 */
const int DELAY = 17;

/**
 * The maximum number of bars that can horizontally fit on the MicroView
 */
const int MAXBARS = XMAX / BARWIDTH;

void draw_bar(int x, int y);
void draw_boxed_number(int n, int x, int y);
int get_microphone_value(int pin);
void draw_waveforms();
void emit(int hz, int pin);
int linear_convert(int value, int oldmin, int oldmax, int newmin, int newmax);

/**
 * Converts a value from one coordinate plane to its equivalent value on another coordinate plane
 * @param value The value to convert
 * @param oldmin The minimum value of the old coordinate plane
 * @param oldmax This maximum value of the old coordinate plane
 * @param newmin The minimum value of the new coordinate plane
 * @param newmax This maximum value of the new coordinate plane
 */
int linear_convert(int value, int oldmin, int oldmax, int newmin, int newmax)
{
	return (oldmax - oldmin) == 0
		? newmin
		: round((value - oldmin) * (newmax - newmin) / (oldmax - oldmin) + newmin);
}

/**
 * Pushes a vertical line onto the draw buffer at the specified position and height
 * @param x The horizontal position of the line
 * @param y The height of the line
 */
void draw_bar(int x, int y)
{
	const int XEND = x + BARWIDTH;

	for(int i = x; i < XEND; i++)
	{
		uView.line(i, YMAX, i, y);
	}
}

/**
 * Pushes a box with a number onto the draw buffer at the specified coordinate
 * @param n The number to display
 * @param x The x (horizontal) coordinate
 * @param y The y (vertical) coordinate
 */
void draw_boxed_number(int n, int x = 1, int y = 1)
{
	uView.setCursor(x, y);

	uView.setColor(1);
	int xStart = x - 1;
	int xEnd = (x - 1) + 13;
	for(int i = xStart; i < xEnd; i++)
	{
		uView.line(i, 0, i, 9);
	}

	uView.setColor(0);
	uView.print((n < 10 ? '0' : '\0') + String(n));
	uView.setColor(1);
}

/**
 * Returns the analog value of the specified pin. The value is clamped to ADCMAX.
 * @param pin The physical pin to read the value from
 */
int get_microphone_value(int pin = A0)
{
	int value = analogRead(A0);

	return value > ADCMAX ? ADCMAX : value;
}

/**
 * Reads the analog input, delegates draw calls, and maintains the data necessary
 * to correctly display subsequent calls of this method.
 */
void draw_waveforms()
{
	const int LASTARRAYINDEX = MAXBARS-1;
	static int xValues[MAXBARS] = {0};
	static int frame = 0;
	int i;
	int pinValue;

	for(i = 0; i < MAXBARS; i++)
	{
		xValues[i] = xValues[i+1];
	}

	pinValue = get_microphone_value(A0);
	xValues[LASTARRAYINDEX] = linear_convert(pinValue, 0, ADCMAX, 0, YMAX);

	for(i = 0; i < MAXBARS; i++)
	{
		// Subtract from YMAX to draw 0 at the bottom, and YMAX at the top of the microview.
		draw_bar(i * BARWIDTH, YMAX - xValues[i]);
	}

	draw_boxed_number(YMAX - xValues[LASTARRAYINDEX], 14);

	frame = frame < LASTARRAYINDEX ? (frame + 1) : 0;
}

/**
 * Outputs a hz signal at 39.2% duty cycle to the specified pin.
 * Sets TCCR2B to the specified hz value.
 * pin3 is PWM0
 */
void emit(int hz = 1, int pin = 3)
{
	pinMode(pin, OUTPUT);
	TCCR2B = hz;
	analogWrite(pin, 100);
}

void loop()
{
	uView.clear(PAGE);

	draw_waveforms();
	emit(1000);

	uView.display();

	#if DELAY != 0
		delay(DELAY);
	#endif
}

void setup()
{
	uView.begin();
	uView.clear(PAGE);
}
