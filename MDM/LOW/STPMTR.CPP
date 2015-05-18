/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */

#define INCL_DOSERRORS
#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSDATETIME
#include <time.h>
#include <dos.h>
#include "hiltner.h"
#include <iostream.h>
#include "stpmtr.h"

extern HMTX  setencoders_mex_sem,steppermotor_mex_sem;
int semix = 0;
/************************  Stepper Motor Low Level  ***********************/
StepperMotor::StepperMotor (PortBit _fwdrev,SquareWave _clk,
														 double _stepsize,int polarity)
 : Motor (STEPSPEED,STEPACCEL)
{
  fwdrev = _fwdrev; clk = _clk;
	stepsize = _stepsize;
	direction_polarity = polarity;
	common ();
}
void StepperMotor::common ()
{

	cur_direction = 1;
	directionsense = 1;
	cur_speed = cur_freq = 0.0;
	stop_freq = 0.0;
	clk.set_frequency (cur_freq);;
}
/**************************  direction   ******************************/
/*
		 set the direction of the stepper motor.
 */
	 void StepperMotor::direction (int _direction)
	 {
		if (_direction*direction_polarity < 0) {
			fwdrev.set_bit ();
		}
		else {
			fwdrev.clear_bit ();
		}
		cur_direction = _direction;
	 }

	 // not used....
 //	 void StepperMotor::set_stop_speed (double _speed) {
 //		 stop_freq = _speed/stepsize;
	// }
/******************************  stop  **********************************/
	 // speed is in arc-seconds/second.  A non-zero speed starts the motor
	 // turning at that speed.  A zero speed stops the motor.
	 // Note: for DC motors setting the speed involves some acceleration and
	 //       deaceleration to reach the desired speed.
	 void StepperMotor::stop (double _speed)
	 {
		 speed_at_direction (-_speed,cur_direction);
	 }
/*************************  stop at direction  **************************/
/*
		 Take out a contribution to motion from one of the independent processes.
	 If only one process told the motor how fast to move, this routine would
	 stop the motor.
 */
	 void StepperMotor::stop_at_direction (double _speed,int _direction)
	 {
		 speed_at_direction (-_speed,_direction);

	 }
/******************************  speed *********************************/
/*
		 sets the motor in motion by calculating a frequency and setting
	 the pulse rate of the motor on the counter timer card.
 */
	 void StepperMotor::speed (double _speed) {
			// speed is in arcseconds/sidereal seconds.
			// convert to real seconds and then to the appropriate frequency.
			cur_freq =(_speed*ADJUSTEDSIDEREALRATE)/stepsize;


/*

char buf[256];
sprintf (buf,"freq:%lf  speed:%d  stopfreq:%lf  dir:%d",
						cur_freq,_speed,stop_freq,cur_direction);
debug.post (buf);
*/
/*
char buf[256];
sprintf (buf,"stp size:%lf freq:%2.15lf +spd:%lf speed:%lf sense:%d",
					stepsize,cur_freq,_speed,cur_speed,directionsense);
debug.post (buf);
*/
			clk.set_frequency (cur_freq);
	 }
/****************************** estimate speed *********************************/
/*
		 sets the motor in motion by calculating a frequency and setting
	 the pulse rate of the motor on the counter timer card.
 */
	 double StepperMotor::estimate_speed (double _speed) {
			// speed is in arcseconds/sidereal seconds.
			// convert to real seconds and then to the appropriate frequency.
			double est_cur_freq =(_speed*ADJUSTEDSIDEREALRATE)/stepsize;


			return clk.get_frequency (est_cur_freq)*stepsize/ADJUSTEDSIDEREALRATE;
	 }
	 double StepperMotor::estimate_speed_at_direction (double _speed, int _direction) {

		 if (_speed == 0.0){

				return 0.0;
		 }

				if (_direction > 1) _direction = 1;
				else if (_direction < 1) _direction = -1;
// REL seems skrewy...
				else if (_direction == 0) {
//					// get direction from speed.
					//if (_speed < 0) _direction = -1;
					//else
          _direction = 1;
				}

				double est_cur_speed = _speed*_direction;
				double est_directionsense;
				if (est_cur_speed < 0)
					est_directionsense = -1;
				else
					est_directionsense = 1;

				// speed is in arcseconds/second.
				if (est_cur_speed > 150) est_cur_speed = 150;

				//direction (est_directionsense);
				if (est_cur_speed == 0.0)
					return 0.0; //speed (0.0);
				else
					return estimate_speed (est_cur_speed*est_directionsense);

			}


/****************************  speed at direction  ************************/
/*
			Since several processes are independently setting stepper motor speeds,
	 and since each movement process motions must be combined, this routine
	 adds and subtracts speeds from the current stepper motor speed.
*/
	 double StepperMotor::speed_at_direction (double _speed, int _direction) {

		 if (_speed == 0.0){

				return _speed;
		 }

				if (_direction > 1) _direction = 1;
				else if (_direction < 1) _direction = -1;
				else if (_direction == 0) {
					// get direction from speed.
// REL seems skrewy  converts speed to positive value!
// probably shoud be speed <= abs (speed) and direction set
// appropriately, but parameters don't translate to the outside.
					//if (_speed < 0) _direction = -1;
					//else
// REL end modify 4-16-98
					_direction = 1;
				}

				cur_speed += _speed*_direction;

				if (cur_speed < 0)
					directionsense = -1;
				else
					directionsense = 1;

				// speed is in arcseconds/second.
				if (cur_speed > 150.0) return 0.0;

				direction (directionsense);
				if (cur_speed == 0.0)
					speed (0.0);
				else
					speed (cur_speed*directionsense);
				return _speed;
			}

/**************************  change speed at direction  **********************/
/*
			Since several processes are independently setting stepper motor speeds,
	 and since each movement process motions must be combined, this routine
	 adds and subtracts speeds from the current stepper motor speed.
*/
	 double StepperMotor::change_speed_at_direction (double _oldspeed,
							 double _olddirection,double _speed, int _direction) {

	//	 if (_speed == 0.0){
	//			return _speed;
	//	 }
				if (_olddirection > 1) _olddirection = 1;
				else if (_olddirection < 1) _olddirection = -1;
				else if (_direction == 0) {
					// get direction from speed.
// REL seems skrewy... takes the absolute value of speed when
// calculating new speed...
 //					if (_speed < 0) _olddirection = -1;
//					else
// REL end change 4-16-98
					_olddirection = 1;
				}
				cur_speed -= _oldspeed*_olddirection;
				if (_direction > 1) _direction = 1;
				else if (_direction < 1) _direction = -1;
				else if (_direction == 0) {
					// get direction from speed.
// REL seems skrewy... takes the absolute value of speed when
// calculating new speed...
				 //if (_speed < 0) _direction = -1;
				 //else
					_direction = 1;
// REL end change 4-16-98
				}

				cur_speed += _speed*_direction;

				if (cur_speed < 0)
					directionsense = -1;
				else
					directionsense = 1;

				// speed is in arcseconds/second.
				if (cur_speed > 150.0) return 0.0;

				direction (directionsense);
				if (cur_speed == 0.0)
					speed (0.0);
				else
					speed (cur_speed*directionsense);
				return _speed;
			}

/*
	distance = arcseconds
	speed = steps/seconds
	direction -1 or 1.
 */
/*******************************  move  *******************************/
/*
			Move moves the stepper motor a specific distance at a specific
	 speed and direction.  The procedure uses a timing loop to move the
	 appropriate amount.
 */
int StepperMotor::move (double _distance,double _speed,int _direction)
{
	// distance given in arcseconds and converted to steppermoter counts.
	// speed given in arcseconds per second.
	// calculate # seconds of activity to move required amount.

	double fsecs = _distance/_speed * CLK_TCK;
	// the unit of time_t is seconds.
	clock_t starttime,stoptime;
 //	direction (_direction);
 //	if (DosRequestMutexSem(steppermotor_mex_sem,1000L) != ERROR_TIMEOUT) {


		 speed_at_direction (_speed,_direction);
		// wait for the telescope to move the prescribed amount.
							char buf[256];
	//	DosReleaseMutexSem (steppermotor_mex_sem);
		starttime=clock ();
		stoptime = clock ();
		//					 sprintf (buf," length:%lf speed:%lf dir:%d sec:%lf %ld %ld %ld",
		//						_distance,_speed,_direction,fsecs,starttime,stoptime,stoptime-starttime);
 //	debug.post (buf);
		do {
			stoptime = clock ();

		}
		while (stoptime-starttime < (long)fsecs);
							// sprintf (buf," length:%lf speed:%lf dir:%d sec:%lf %ld %ld %ld",
							//	_distance,_speed,_direction,fsecs,starttime,stoptime,stoptime-starttime);
		//debug.post (buf);

		stop_at_direction (_speed,_direction);// speed adds in a stop speed value.
		return 1;
 //	}
 //	return 0;
}
/*******************************  move  *******************************/
/*
			Move moves the stepper motor a specific distance at a specific
	 speed and direction.  The procedure uses a timing loop to move the
	 appropriate amount.
 */
extern int gcoordination;
int StepperMotor::sync_move (double _distance,double _speed,int _direction,int &coordination)
{
	// distance given in arcseconds and converted to steppermoter counts.
	// speed given in arcseconds per second.
	// calculate # seconds of activity to move required amount.

	double fsecs = _distance/_speed * CLK_TCK;
	// the unit of time_t is seconds.
	clock_t starttime,stoptime;
 //	direction (_direction);
 //	if (DosRequestMutexSem(steppermotor_mex_sem,1000L) != ERROR_TIMEOUT) {
	 /*	 gcoordination++;
		 do { // wait for two post events.
			 //	DosQueryEventSem (mytrailevent,&posted);
			 //	sprintf (buf,"%s posted:%d %d",data->axis->which,posted,trailpost);
				// debug.post (buf);
		 }
		 while (gcoordination < 2);*/
		 if (_distance != 0.0 && _speed != 0.0) {
			 speed_at_direction (_speed,_direction);

		// wait for the telescope to move the prescribed amount.
					 //		char buf[256];
	//	DosReleaseMutexSem (steppermotor_mex_sem);
			starttime=clock ();
			stoptime = clock ();
		//					 sprintf (buf," length:%lf speed:%lf dir:%d sec:%lf %ld %ld %ld",
		//						_distance,_speed,_direction,fsecs,starttime,stoptime,stoptime-starttime);
 //	debug.post (buf);
			do {
				stoptime = clock ();

			}
			while (stoptime-starttime < (long)fsecs);
							// sprintf (buf," length:%lf speed:%lf dir:%d sec:%lf %ld %ld %ld",
							//	_distance,_speed,_direction,fsecs,starttime,stoptime,stoptime-starttime);
		//debug.post (buf);

			stop_at_direction (_speed,_direction);   // speed adds in a stop speed value.
		}
		return 1;
 //	}
 //	return 0;
}

void StepperMotor::dout ()
{
	cerr <<"[stepper motor: "<<stepsize<<endl;
	clk.dout ();
	fwdrev.dout ();
	cerr <<"]"<<endl;
}

