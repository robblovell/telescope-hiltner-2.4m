/* -------------------------------------------------------------------
Michigan Dartmouth MIT Hiltner 2.4 Meter Telescope
Telescope Control System
By Robb E. Lovell
------------------------------------------------------------------- */
/*
															Dcmotor.cpp

			This file contains low level code for manipulating a dc motor.
	A dc motor is moved by applying a voltage through the dt2816 digital to
	analog converter card.
	axises from the paddle.
*/
#include <iostream.h>
#include "dcmotor.h"
#include <time.h>
#include <conio.h>
/*******************  DCMotor constructors  ***************************/

  DCMotor::DCMotor (DT2816 *_AD_board,int _dac,PortBit _fwdrev)
	 : Motor (DCSPEED,DCACCEL)
  {
	 AD_board = _AD_board;dac = _dac; fwdrev = _fwdrev;
	 type = RA_OR_DEC; common ();
  }
  DCMotor::DCMotor (DT2816 *_AD_board,int _dac,PortBit _fwd,PortBit _rev)
	 : Motor (DCSPEED,DCACCEL)
  {
	 AD_board = _AD_board;dac = _dac; fwd = _fwd; rev = _rev;
	 type = DOME; common ();
  }
  void DCMotor::common () {

	 cur_speed = 0;
	 AD_board->generate_volts (dac,cur_speed);
	 stop ();

	}
/****************************  match speed  *****************************/
/*
		check to see if the right speed has been reached.
 */
int matchspeed (double _speed,double volts, double cur_speed) {
	if (_speed > cur_speed)
		return volts < _speed;
	else
		return volts > _speed;
}
/*****************************  speed  **********************************/
/*
			Ramps up the voltage on a dc motor.
 */
void DCMotor::speed (double _speed)
	{

	 clock_t start_time,stop_time;

	 //  ramps up the motor to a  certain speed...
		direction ();
		start_time = clock ();
		double voltsinc;
		//accelerate or deccelerate?
		if (_speed > cur_speed) voltsinc = DCACCEL_DCRES;
		else voltsinc = -DCACCEL_DCRES;
		// start at the current speed.
		double volts = cur_speed;
		AD_board->generate_volts (dac,volts);

//		cerr <<"SLEW SPEED TO "<<_speed<<"  from:"<<cur_speed<<endl;

 //char buf[256];
 //sprintf (buf,"speed:%lf",volts);
 //debug.post (buf);
//(_speed > cur_speed?(volts < _speed):(volts > _speed))
		while (  matchspeed (_speed,volts,cur_speed) &&
						 volts <= max_speed) {
			stop_time = clock ();
			if ((stop_time-start_time) > DCRES_INV*CLK_TCK) {
			 volts += voltsinc;
			 AD_board->generate_volts (dac,volts);
//debug3 = volts;
			 start_time = clock ();
			}
		}

		AD_board->generate_volts (dac,_speed);
		cur_speed = _speed;
//debug3 = cur_speed;
		if (cur_speed == 0.0) stop_dome (); // the dome requires that the fwd & rev
												 // ttl lines be set to 0v.
	}

/***********************  start speed change  *****************************/
/*
			start the timer for a speed change.
 */
	void DCMotor::start_speed_change ()
	{
		start_time = clock ();
	}
/***********************  change speed  ***********************/
/*
		 Slow down or speed up to the given speed.  Assumes that the
		start_speed_change has been called.
 */
	void DCMotor::change_speed (double _speed) // increment toward given speed.
	{
		//  ramps the motor toward a certain speed...
		direction ();
		double voltsinc;
		//accelerate or deccelerate?
		if (_speed > cur_speed) voltsinc = DCACCEL_DCRES;
		else voltsinc = -DCACCEL_DCRES;
		// start at the current speed.
		double volts = cur_speed;
		AD_board->generate_volts (dac,volts);

		cerr <<"SLEW SPEED TO "<<_speed<<"  from:"<<cur_speed<<endl;
		if ( ! ((_speed > cur_speed?(volts < _speed):(volts > _speed)) && volts < max_speed)) {
			stop_time = clock ();
			if ((stop_time-start_time) > DCRES_INV*CLK_TCK) {
			 volts += voltsinc;
			 AD_board->generate_volts (dac,volts);
			 start_time = clock ();
			}
		}
		else {
			AD_board->generate_volts (dac,_speed);
			volts = _speed;
		}
		cur_speed = volts;
		if (cur_speed == 0.0) stop_dome (); // the dome requires that the fwd & rev
												 // ttl lines be set to 0v.
	}
