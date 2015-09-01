
#ifndef __API_H__
#define __API_H__

#include "i2c.h"

/** list of presence sensors connected to the Arduino board */
enum psensor {
	/** the first presence sensor on the left */
	PSENSOR_A = 0x1,
	/** the second presence sensor on the left */
	PSENSOR_B = 0x2,
	/** presence sensor on the middle */
	PSENSOR_C = 0x4,
	/** the second sensor of the right */
	PSENSOR_D = 0x8,
	/** the last sensor (on the right) */
	PSENSOR_E = 0x10
};

/** state of (all) presence sensors when there is no object */
#define NO_DETECTED_OBJECT 31

/** store ball position */
struct position {
	/** x coordinate of the ball */
	int x;
	/** y coordinate of the ball */
	int y; /* y coordinate of the ball */
};


/** buffers storing I2C data */
extern u8 i2c_data[I2C_PORT_N][I2C_DATA_N];

/** NXT port connected to Arduino board */
int arduino_port;

/** global state of presence sensors */
int pstate = NO_DETECTED_OBJECT;

/** Initiate Arduino driver */
void init_camduino(int);

/** Fill a position strucuture to get ball position */
void get_ball_position(struct position*, int);

/** Get global presence sensors state, consider the five sensors as one */
int object_detected(int);

/** get an given presence sensor state */
int get_pstate(enum psensor);

void get_distance(int *, int);

#endif // __API_H__
