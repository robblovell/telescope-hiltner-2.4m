#ifndef telescopehh
#define telescopehh

#include <iostream.h>
#include <math.h>
#include "const.h"
#include "dtboard.h"
#include "portbit.h"
#include "motor.h"
#include "dcmotor.h"
#include "stpmtr.h"
#include "encoder.h"
#include "spdstat.h"
#define STOP		0
#define TRACK 	1
#define GUIDE	2
#define FAST    4
#define SET		8
#define OFFSET	16

#define TRAIL		32
#define SLEW	64
#define LEFT	1
#define RIGHT -1
#define EAST	1
#define WEST	-1
#define NORTH 1
#define SOUTH -1
#define FORWARD 1
#define REVERSE -1
#define MAXSLEWSPEED		8
#define LIMITSSLEWSPEED	1


//dummy variable used temporarily to look at some values.  No longer used...
extern int debug1,debug2,debug3,debug4;
/**************************  Limits *********************************/
#define HARAWLIMIT	6.0
#define HARAWLIMITREFLECT6	6.0
class Limits {
	public:
		Switch fwd5,rev5,fwd8,rev8; //limit switches.
		double revmax,fwdmax,maxslew,minslew;
		int limitstate;
		int softwarelimit;
		Limits (DTBoard *board,unsigned int port,
			 unsigned int bit5f,unsigned int bit5r,unsigned int bit8f,unsigned int bit8r,
					 double fwd,double rev,double _minslew,double _maxslew);
		void readstatus ();
		// 0 - within limits, 5 outside five degree limits,
	// 8 outside 8 degree limits.
	int limits (double location,int direction);
	int limits (int direction);
	int limits ();


	int inside_softlimits (double location);
	int outside_softlimits (double location);


	double maxspeed (double location,int direction);
	double maxspeed (int direction);
	void externallimit (int reached_limit);
};// limit switches.
/*
		 The procedures below are not encapselated with limits class.  The
	reason for this is that the limits class only corresponds to one axis,
	where these functions reguire data from both axis.
*/
extern int inside_rawlimits (double rawha,double rawdec);
extern int inside_ra_ambiguous_zone (double targetha);
/*******************************  Axis  ***********************************/
class Axis {
	public:
	Control fwdrev;
	SquareWave clockfout;
	Control clutch;
	Control slewenable;
	Control stepenable;
	Control dcfwdrev;
	DCMotor dcmtr;
	StepperMotor stepmtr;

	double slewspeed,maxslewspeed;
	double trackspeed,setspeed,guidespeed,fastspeed,offsetspeed,trailspeed,ratecorr;

	double moveminslewspeed,moveslowslewspeed,
					movehalfslewspeed,movemaxslewspeed;
	double movesetspeed,moveguidespeed;
	double traillength;
	double target;
	char which[5];
	int clutchstate,slewstate,moving,stepperdirection,stepstate;
	int clutchdelay;
	int moveflag,offsetmoveflag,trailflag;

	Limits limit; //limit switches.

	Axis  ( DT2819 *ctboard,DT2816 *adboard,unsigned int dac,int cd,
				 DT2819 *limitboard,int limitbit5f,int limitbit5r,
				 int limitbit8f,int limitbit8r,
				 double minslew,double maxslew,
				 DT2819 *clockboard,int clocksource,
				 double _trackspeed,double speedfactor,double stepsize,double fwdmax,
				 double revmax,int polarity,char *_which,

				 double s1,double s2,double s3,double s4,double s5,double s6);


	void engage_clutch ();
	void disengage_clutch ();
	void slew (int direction);
	void slew_slowup (int byspeed);
	void slew_stop ();
	void stop ();

	void check_limits (double location);
	void slewdirection (int direction);
	void stepdirection (int direction);
	void changeslew (double _speed);
	void startslew ();

	~Axis () {}
};
/******************************  Process Data  *****************************/
/*
		ProcessData data structure is used to define the paddle switches and motors
	that are used to manipulate either the RA or the DEC arms of the telescope.

	Note that the variables reference already defined variables that exist in a
	global "blackboard" scope.
 */
class ProcessData {
	public:
	 Axis *axis;
	 Switch *left,*right;
	 Switch *guideleft,*guideright;
	 Encoder *encoder;
	 SpeedStatus *status;
	 int axismoveflag;
	 int leftlimitsflag,rightlimitsflag;
	 ProcessData (Axis &a,Switch &l,Switch &r,Switch &gl,Switch &gr,
								Encoder &e,SpeedStatus &stats)
	 {
		 axismoveflag = 0;
		 axis = &a; left = &l; right = & r;
		 guideleft = &gl; guideright = &gr; encoder = &e;  status = &stats;
		 leftlimitsflag = 0; rightlimitsflag = 0;
	 }
	 ~ProcessData () {}
};
extern ProcessData RAdata;
extern ProcessData DECdata;
/*********************************  Dome  *********************************/
/*
			This class handles the dome control functions which move the dome.
	 The intelligence for responding to telescope motions is performed in
	 the dome process in dome.cpp.  This class handles motion control only.
 */
#define OUTPUT	0
#define INPUT	1
#define CW	1
#define CCW -1
// dome
class Dome {
 public:
	Control ccw,cw,enable;
	DCMotor mtr;
	double maxspeed;
	int domestate;
	Encoder *encoder;
	Dome (DTBoard *board1,DTBoard *board3,DT2816 *adboard,Encoder *enc);
	~Dome () {}
	void speedcheck (double &speed);
	void move_ccw (double speed);
	void move_cw (double speed);
	void move_distance (double speed,double seconds,int domedirection);

	void move_distance (double distance,int domedirection,double &speed,double &volts,double &seconds);
	void move (double speed,int direction);
	double calculate_speed_n_seconds (double distance,double &volts,double &seconds);

	void stop ();

};

/********************************  Paddle  ******************************/
/*
		 Keeps track of the state of the paddle switches.
*/
class Paddle {
	public:
		Switch N,S,E,W,guide_set,focusin,focusout;
		PushSwitch slew;

		int slewclicks;

		Paddle (DTBoard *board)
			:		N ((DTIO*)board,0,0,ACTIVELOW),
			S ((DTIO*)board,0,1,ACTIVELOW),
			E ((DTIO*)board,0,2,ACTIVELOW),
			W ((DTIO*)board,0,3,ACTIVELOW),
			guide_set ((DTIO*)board,0,4,ACTIVELOW),
			slew ((DTIO*)board,0,5,ACTIVELOW),
			focusin ((DTIO*)board,0,6,ACTIVELOW),
			focusout ((DTIO*)board,0,7,ACTIVELOW)
		{}
		void readstatus ()
		{
			N.read_state ();
			S.read_state ();
			E.read_state ();
			W.read_state ();
			guide_set.read_state ();
			slew.read_state (); //slewclicks);
			focusin.read_state ();
			focusout.read_state ();
		}
};
/*******************************  Front Panel  ****************************/
/*
			Keeps track of the state of the front panel switches.
 */
class FrontPanel {
	public:
		Switch drivesonoff_,domehome_,autodome_,autoguide_,go_,track_,executego_,stop_;	// Front Panel
		FrontPanel (DTBoard *board2,DTBoard *board3);
		~FrontPanel () {}
		void readstatus ();
		int drives ();
		int autodome ();
		int autoguide ();
		int track ();

};
/*******************************  Auto Guide  ***************************/
/*
		 Keeps track of the state of the auto guide switches.
 */
class AutoGuide {
	public:
		Switch N,S,E,W,guide_fast;
	AutoGuide (DTBoard *board)
		: N ((DTIO*)board,1,2,ACTIVELOW), S ((DTIO*)board,1,3,ACTIVELOW),
			E ((DTIO*)board,1,4,ACTIVELOW), W ((DTIO*)board,1,5,ACTIVELOW),
			guide_fast ((DTIO*)board,1,6,ACTIVELOW)
	{}
	~AutoGuide () {}
	void readstatus ()
	{
			N.read_state ();
			S.read_state ();
			E.read_state ();
			W.read_state ();
			guide_fast.read_state ();
	}
};

	// focus direction.
#define FOCUSIN	1
#define FOCUSOUT	-1
class Focus {
 public:
	Switch fwdlimit,revlimit;
	Control fwdrev,enable,fast_guide;
	Encoder *encoder;
	double fastspeed,guidespeed;	double stepspeed;
	Focus (DTBoard *board2, Encoder *enc);
	~Focus () {}

	void focusin ();
	void focusout ();
	void stop ();
	void set_guide ();
	void set_fast ();
};


#endif
