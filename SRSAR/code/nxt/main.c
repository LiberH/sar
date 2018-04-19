
#include "tpl_os.h"
#include "i2c.h"
#include "api.h"

FUNC(int, OS_APPL_CODE) main(void) {
    /* Motors: */
    nxt_motor_set_speed(NXT_PORT_A, 100, TRUE);
	nxt_motor_set_speed(NXT_PORT_B, 0, TRUE);
    
    /* I2C: */
    enum i2c_conf conf[] = {I2C_CONF_GETDATA};
    
    i2c_init();
    nxt_avr_set_input_power(NXT_PORT_S3, LOWSPEED);
    nxt_avr_set_input_power(NXT_PORT_S4, LOWSPEED);
    i2c_start(NXT_PORT_S3, conf, 0);
    i2c_start(NXT_PORT_S4, conf, 0);

    /* OS: */
    StartOS(OSDEFAULTAPPMODE);
	return 0;
}

ISR(StopISR) {
    /* Motors: */
    nxt_motor_set_speed(NXT_PORT_A, 0, TRUE);
	nxt_motor_set_speed(NXT_PORT_B, 0, TRUE);
    
    /* I2C: */
    i2c_stop(NXT_PORT_S3);
    i2c_stop(NXT_PORT_S4);
    
    /* OS: */
    ShutdownOS(E_OK);
}

DeclareTask(Task);
DeclareTask(Display);

// ----------

char detected = FALSE;
int distance = 0;
struct position ball = {0, 0};
int speed = 0;

TASK(Task) {
    
    /*******************/
    /* Racket control: */
    /*******************/
    detected = object_detected(NXT_PORT_S4);
    if (detected) {
        /* Fire: */
        nxt_motor_set_speed(NXT_PORT_A, -100, 1);
	    while (!ecrobot_get_touch_sensor(NXT_PORT_S2));
        
        /* Rearm: */
	    nxt_motor_set_speed(NXT_PORT_A, 100, 1);
    }
    /*******************/
    
    /******************/
    /* !!!! TODO !!!! */
    /* Wheel control: */
    /* !!!! TODO !!!! */
    /******************/
    get_distance(&distance, NXT_PORT_S3);
    get_ball_position(&ball, NXT_PORT_S4);
    speed = 0.9 * (60 - ball.y);
    if (speed >  100) speed =  100;
    if (speed < -100) speed = -100;
    
    if (ball.y)
        ecrobot_set_motor_speed(NXT_PORT_B, speed);
    /******************/
    
	TerminateTask();
}

TASK(Display) {
    display_clear(FALSE);
    
    display_goto_xy(0, 0); display_string(detected ? "Something" : "Nothing");
    display_goto_xy(0, 1); display_string("x = "); display_int(ball.y, 4);
    display_goto_xy(0, 2); display_string("y = "); display_int(ball.x, 4);
    display_goto_xy(0, 3); display_string("Dist.: "); display_int(distance, 4);
    display_goto_xy(0, 4); display_string("Speed: "); display_int(speed, 4);
    
	display_update();
	TerminateTask();
}