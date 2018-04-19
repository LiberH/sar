
#include "api.h"
#include "nxt_avr.h"
#include "ecrobot_interface.h"

/** 
 * Initiate Arduino driver, don't forget to call i2c_init() after.
 * Set up i2c port, tram communication and *power* supply.
 * @param[in] i2c_port NXT port where Arduino is connected
 */
void init_camduino(int i2c_port){
	arduino_port = i2c_port;
	enum i2c_conf conf[] = {I2C_CONF_GETDATA};
	nxt_avr_set_input_power(arduino_port, 2);
	i2c_start(arduino_port, conf, 0);
}

/**
 * Fill a position strucuture to get ball position
 * @param[in] ball A reference to the struct to fill 
 */
void get_ball_position(struct position *ball, int arduino_port){
	/* get ball position from I2C */
	ball->x = i2c_data[arduino_port][1] << 8;
	ball->x |= i2c_data[arduino_port][0];
	
	ball->y = i2c_data[arduino_port][3] << 8;
	ball->y |= i2c_data[arduino_port][2];
}

/**
 * Get global presence sensors state 
 * @param[out] int State of the presence sensors, equals to NO_DETECTED_OBJECT if 
 * an object is not detected, else return a different value
 */
int object_detected(int arduino_port){
	return 0x1f & ~i2c_data[arduino_port][4];
}

/**
 * Get an given presence sensor state
 * @param[in] sensor Name of the presence sensor to examine
 * @param[out] int state of the sensor, return 0 if no detected object, else 1
 */
int get_pstate(enum psensor sensor){
  return !(i2c_data[arduino_port][4] & sensor);
}

void get_distance(int *distance, int arduino_port){
    *distance = i2c_data[arduino_port][1] << 8;
	*distance |= i2c_data[arduino_port][0];
}
