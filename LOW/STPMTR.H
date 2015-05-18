#ifndef steppermotorhh
#define steppermotorhh

#include "motor.h"
#include "portbit.h"

#define	STEPSPEED   100      // arc-sec/sec
#define 	STEPACCEL   -1		// instantaneous
extern int semix;
class StepperMotor: public Motor {
  public:
	 PortBit fwdrev;
	 SquareWave clk;
	 double stepsize;  // conversion from arc-seconds to motor steps.
	 double cur_freq,stop_freq,cur_speed; // in pulses/second (motor steps)
	 int cur_direction,directionsense;
	 HMTX  steppermotor_mex_sem;
	 char sem[50];
	 // direction polarity is used to figure out which way to add the stop_speed.
	 // if stop is non-zero, then one direction will have a positve stop speed
	 // and the other a negative stop speed.
	 int direction_polarity; // change which way is the positive direction, 1 or -1.

	 StepperMotor () {}
	 StepperMotor (PortBit _fwdrev,SquareWave _clk,double _stepsize,int polarity);
	 ~StepperMotor () {
	 }
	 void common ();
	 void stop (double _speed);
	 void stop_at_direction (double _speed,int _direction);
	 // direction should set which way the motor turns.
	 virtual void direction (int _direction);
	 //void set_stop_speed (double _speed);
	 // speed is in arc-seconds/second.  A non-zero speed starts the motor
	 // turning at that speed.  A zero speed stops the motor.
	 // Note: for DC motors setting the speed involves some acceleration and
	 //       deaceleration to reach the desired speed.
	 virtual void speed (double _speed);
	 virtual double speed_at_direction (double _speed,int direction);
	 double change_speed_at_direction (double _oldspeed,
							 double _olddirection,double _speed, int _direction);

	 // Move moves the motor a specified distance at a specifed speed in a
	 // specified direction.  distance is in arc-seconds, speed in
	 // arc-seconds/second, and direction is -1 for reverse and +1 for
	 // forward.  Note this is a blind move...
	 virtual int move (double _distance,double _speed,int _direction);
	 double estimate_speed (double _speed) ;

	 double estimate_speed_at_direction (double _speed,
									int _direction);
	 int sync_move (double _distance,double _speed,int _direction,int &coordination);

	 // I/O routines for this object.
	 friend ostream &operator << (iostream &os, Motor &me) ;
	 void dout ( );

};
#endif
