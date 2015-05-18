/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/********************************  motor.h  *******************************/
#ifndef motorhh
#define motorhh

#include <iostream.h>
/*****************************  Motor Class  ******************************/
/*
		 The motor data class is a base class for all motor type
  classes.  A motor is defined as a piece of hardware which can move
  rotationally in two directions at a given speed.  The hardware is
  limited to a maximum speed and acceleration.  No limit is placed
  upon the number of revolutions that can be made.

		 Note that the functions are defined as pure virtual functions,
  meaning that any type of motor class which uses this class as a base
  must define these functions.
 */
class Motor {
  public:

	 double max_speed;
	 double max_acceleration;

	 Motor ()
	 {
		max_speed = 0;
		max_acceleration = 0;
	 }

	 Motor (double _max_speed,double _max_acceleration)
	 {
		max_speed = _max_speed;
		max_acceleration = _max_acceleration;
	 }
	 ~Motor () {}
	 // assignment functions.
	 void set (double _max_speed,double _max_acceleration)
	 {
		max_speed = _max_speed;
		max_acceleration = _max_acceleration;
	 }
	 // pure virtual functions.
	 // direction should set which direction the motor turns in arc seconds.
	 virtual void direction (int _direction)=0;
	 // speed is in arc-seconds/second.  A non-zero speed starts the motor
	 // turning at that speed.  A zero speed stops the motor.
	 // Note: for DC motors setting the speed involves some acceleration and
	 //       deaceleration to reach the desired speed.
	 virtual void speed (double _speed)=0;
	 // Move moves the motor a specified distance at a specifed speed in a
	 // specified direction.  distance is in arc-seconds, speed in
	 // arc-seconds/second, and direction is -1 for reverse and +1 for forward.
	 virtual int move (double _distance,double _speed,int _direction)=0;
	 // I/O routines for this object.
	 friend ostream &operator << (iostream &os, Motor &me) ;
	 void dout ( );

};
#endif