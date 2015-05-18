#ifndef dcmotorhh
#define dcmotorhh
#include "motor.h"
#include "portbit.h"





#define DCSPEED	11   // max speed in volts displacement
#define DCACCEL	20   // acceleration in volts/sec (speed to increase voltage.
#define DCRES		10    // number of increses in voltage/sec
#define DCRES_INV	(1.0/DCRES)
#define DCACCEL_DCRES 	(DCACCEL/DCRES)  // 2

#define DOME		2
#define RA_OR_DEC	1

class DCMotor : public Motor {
  public:
  int dac; // which digital to analog converter:0-3.

  DT2816 *AD_board; // pointer to initialized dt2816 board.
  //  if a counter timer board is used, then port can be 0 or
  // 1 and bits 0-7.  If the digital I/O board is used,
  // the board is 5 and the ports 0-3.
  PortBit fwdrev;
  PortBit fwd,rev;
  // used for different way of controlling forward and reverse.
  int type;     // RA_OR_DEC or DOME (1 or 2).

  double cur_speed;
  int cur_direction;
  clock_t start_time,stop_time;

  DCMotor () {}
  DCMotor (DT2816 *_AD_board)
  {
	 AD_board = _AD_board;
	 common ();
  }
  DCMotor (DT2816 *_AD_board,int dac,PortBit _fwdrev);        // RA or DEC Slew motors.
  DCMotor (DT2816 *_AD_board,int dac,PortBit _fwd,PortBit _rev);  // Dome Motor.
  void common ();
  virtual void speed (double _speed);
  void start_speed_change ();
  void change_speed (double _speed); // increment toward given speed.

  void set_speed (double _speed)
  {
	 cur_speed = _speed;
  }
  void set_direction (int _direction)
  {
	 cur_direction = _direction;
  }
  void direction ()
  {
	 direction (cur_direction);
  }
  virtual void direction (int _direction)
  {

	 if (type == RA_OR_DEC) {
		  if (_direction < 0) {
			 fwdrev.set_bit ();
		  }
		  else {
			 fwdrev.clear_bit ();
		  }
	 }
	 else if (type == DOME) {
		  if (_direction < 0) {
			 rev.clear_bit ();
			 fwd.set_bit ();
		  }
		  else {
			 fwd.clear_bit ();
			 rev.set_bit ();
		  }
	 }
	 cur_direction = _direction;
  }
  void stop_dome ()
  {
  	  if (type == DOME) {
		 rev.clear_bit ();
		 fwd.clear_bit ();
	  }
  }
  void stop ()
  {
	  // kill any threads that are ramping motor speed,
	  // then set the speed to zero.
	  speed (0.0);
	  cur_speed = 0;
  }
	virtual int move (double _distance,double _speed,int _direction)
  {
	 return 0;
  }
};
#endif