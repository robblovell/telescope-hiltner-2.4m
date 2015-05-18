/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/*******************************  encoder.h  ***************************/
#ifndef encoderhh
#define encoderhh
/*
*/
#include <iostream.h>
#include "dtboard.h"
#define	COUNTER1		1
#define	COUNTER2		2
#define	COUNTER3		3
#define	COUNTER4		4
#define	COUNTER5		5

#define	BOARD1		1
#define	BOARD2		2
#define	BOARD3		3

#define	BOARD_ADDRESS_1   0x230
#define	BOARD_ADDRESS_2   0x250
#define	BOARD_ADDRESS_3	0x290

#define	FORWARD_LO		0
#define	REVERSE_LO		2

#define	FORWARD_HI		1
#define	REVERSE_HI		3

#define 	LOW_RES		16
#define	HIGH_RES		32

#define RES32	4294967296  //0-7
#define RES16	65536
#define COUNTERRES32	32
#define COUNTERRES16	16   //0-7
#define	CASCADE12	12
#define	CASCADE23	23
#define	CASCADE34	34
#define	CASCADE45	45
/*****************************  counter class  ******************************/
/*
		serves no purpose... But it could have!
 */
class Counter {
	public:
	Counter () {}
};
/******************************  Encoder Class  ******************************/
/*
 */
class Encoder : public Counter {
	public:
	// an encoder can be continous or bound.
	// bound encoders have a min and max (range = max-min+1),
	// continuous encoders have a range (min=0 & max = range-1).

	double min, max, range;

	 /*
			Each encoder can move in two directions, forward and
	 reverse and two or four counters to keep track of the motion.
	 When two counters are used, the resolution is 16 bits, and one
	 counter is used for forward and the other for reverse.  When
	 four counters are used, the resolution is 32 bits, and two
	 counters are cascaded for one direction, two for the other.
	*/
	unsigned long fwd,rev; // Each encoder can move in both directions.
	unsigned long fwdroll,revroll;
	double rawlocation,encoderlocation,arcseclocation,location;
	double rawoffset,locationoffset,arcsecoffset,encoderoffset;
	int keeppositve;
	// short fwdroll,revroll;
	int count[4];		  // up to four counters to digitize 1 encoder.
	int resolution;// counter resoution=16 for 16 bit, resolution=32 for 32 bit.
	double cntrange;    // range of the encoder device.
	double toarcsec;		// scale tor translate encoder counts to arc seconds

  /*
			An encoder can be located on any two of three boards in the system,
	  using up to four of any five counters per board.  Individual
	  directions can be on different boards.  If a encoder is 16 bit,
	  then only one counter is used for each direction.  If an encoder
	  is 32 bit then two counters are used for each direction, either
		counters 2 and 3 or counters 4 and 5. All values in the arrays
	  must be set.  Counters are labeled 1 through 5, boards 1 through 3.
	*/

  DT2819 *board[2];      // 1-3
  int counter[2];    // 1-5

  Encoder (double min_, double max_, int resolution_,
				DT2819 *board_[2],int counter_[2],double toarcseconds)
  {
	 min = min_; max = max; range = max-min+1;
	 common (resolution_,board_,counter_,toarcseconds);
	 // initialize the encoder.
	 init_encoder (resolution,board,counter);
	 // assume last postion of the telescope.

  }

	Encoder (double range_, int resolution_, DT2819 *board_[2],
								 int counter_[2],double toarcseconds);
  Encoder (double range_, int resolution_, DT2819 *board1,DT2819 *board2,
				int counter1,int counter2,double toarcseconds,int _keeppositve);
  Encoder (Encoder &encoder);
  ~Encoder () {
  }
  void set (double min_, double max_, int resolution_,
				DT2819 *board_[2],int counter_[2],double toarcsec_) {
	 min = min_; max = max; range = max-min+1;
	 common (resolution_,board_,counter_,toarcsec_);
	 // initialize the encoder.
	 init_encoder (resolution,board,counter);
	 // assume last postion of the telescope.

  }
	void set (double range_, int resolution_, DT2819 *board_[2],
						int counter_[2],double toarcsec_);

  void init_encoder (int resolution,DT2819 *board[2],int counter[4]);
	void common (int resolution,DT2819 *board_[2],int counter_[4],
											double toarcsec_);
  void init_encoder ();
  void set_to_zero ();
	double read_arcsec_location ();
	double read_raw_location ();
	void set_to_arcsec_location (double loc);
	void set_to_raw_location (double loc);  void read_counters ();
	void read_counters (unsigned long tmp[5]);
	void set_counters (unsigned long tmp[6]);
  int read_fwd ();
  int read_rev ();
	void assign_fwd_n_rev (unsigned long tmp[5]);
	void fwd_n_rev_to_bytes (unsigned long tmp[6]);
	long calculate_location ();
	int calculate_fwd_n_rev (double location);
  void set_range (double min_, double max_) {
	 max = max_;
	 min = min_;
  }
  void set_range (double arange_) {
	 range = arange_;
  }
  /*
			 The following "test_" routines are for testing the validity of
	  the math used for calculating the location of the encoder using
	  the counter timers.  Note that in order for the mat to work, the
	  "cntrange" variable (counter's range) needs to be equal to or a
	  power of 2 greator than the range of the encoder.
  */
  void test_count_up (int upcnt);
  void test_count_dwn (int dwncnt);
  int test_read_location ();
  // I/O routines for this object.
  friend ostream &operator << (iostream &os, Encoder &me) ;
  void dout ( ) ;
};
/************************  continuous Encoder  ***************************/
/*
		 The dome encoder can roll over because the dome can move 360 degrees
	 several times in one direction.  While the condition happening where it
	 does roll over is slight, it must be taken into account.  A continuous
	 encoder can only use counters which have test register for a terminal
	 count so that the counter register counts to value which is modulo of
	 the counts per revolution.
 */
class Continuous_Encoder : public Encoder {
	public:
	Continuous_Encoder (double range_, int resolution_,
											DT2819 *board_[2],int counter_[2],double toarcsec_);
	Continuous_Encoder (double range_, int resolution_, DT2819 *board1,
				 DT2819 *board2,
				int counter1,int counter2,double toarcsec_);
  void init_alarm (double range_);

};
#endif
