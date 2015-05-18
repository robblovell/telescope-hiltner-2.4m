/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/***************************  portbit.h  *******************************/
/*
		 This file contains all the routines that deal with reading and
	writing of the digital i/o ports.  Each class in this file interprets
	the bits of the i/o port in different ways.
 */
#ifndef portbithh
#define portbithh
#include "dtboard.h"
#include <time.h>
/* The define "FOUTBIT" is used to specify a fout type access to the
	counter timer board.
 */
#define	FOUTBIT	-1
// on and off are defined in terms of whether the switch's state is equal
// to its active state.  Thus, active low switches use the same on/off tests.
#define	ON	1
#define OFF 0
#define ACTIVELOW	0
#define ACTIVEHIGH	1
/******************************  PortBit  ********************************/
/*
	Simplest interpretation of an i/o port.

		 Port bit is used to easily specify and use particular bits of the I/O
	subsystem of the DT2819 counter timer board.  A bit is specified to be
	on a opened counter timer board, and from one of two ports labeled 0 and 1.
	Each port can be input or output depending on how the board is initialized.
	The ports are 8 bits wide.
 */
class PortBit {
  public:
	 DTIO *board;
	 unsigned int port,bit;
	 unsigned short state;

	 PortBit () {
	 }
	 PortBit (DTIO *_board,unsigned int _port,unsigned int _bit)
	 {
		 board = _board;
		 port = _port;
		 bit = _bit;
	 }
	 ~PortBit () {}
	 PortBit & operator = (const PortBit &him) {
		 board = him.board;
		 port = him.port;
		 bit = him.bit;
		 state = him.state;

		 return *this;
	 }
	 void clear_bit () {
		 board->clear_bit (this->port,this->bit);
	 }
	 void set_bit () {
		 board->set_bit (this->port,this->bit);
	 }
	 unsigned short get_bit () {
		 return board->get_bit (this->port,this->bit);
	 }
	 void dout ()
	 {
		 cerr <<"[PortBit:board:"<<board->address<<"  port:"<<port<<"  bit:"<<bit<<"]"<<endl;
	 }

};
/********************************  Square Wave  **************************/
/*
		A way of generating square waves either from a counter or the freqency
	out bit depending upon the intialization of the class.
 */
class SquareWave {
	public:
	 DTIO *board;
	 unsigned int source; // -1 is the fout source, 1-5 is counters 1 through 5.
	 double speed;
	 SquareWave () {
	 }
	 SquareWave (DTIO *_board,unsigned int _source,double initial_speed)
	 {
		 board = _board;
		 source = _source;
		 speed = initial_speed;
		 set_frequency (speed);
	 }
	 ~SquareWave () {}
	 SquareWave & operator = (const SquareWave &him) {
		 board = him.board;
		 source = him.source;
		 speed = him.speed;
		 return *this;
	 }
	 void set_frequency (double speed_) {
		 speed = speed_;
		 if (source == FOUTBIT) {
			 board->set_frequency (speed);
		 }
		 else {// here the source is which counter is used to generate the freq.
			 board->set_counter_frequency ((unsigned int)source,speed);
		 }
	 }
	 double get_frequency (double speed_) {

		 if (source == FOUTBIT) {
			 return board->get_frequency (speed_);
		 }
		 else {// here the source is which counter is used to generate the freq.
			 return board->get_counter_frequency ((unsigned int)source,speed_);
		 }
	 }	 void change_frequency (double speed_) {
		 speed = speed_;
		 if (source == FOUTBIT) {
			 board->set_frequency (speed);
		 }
		 else {// here the source is which counter is used to generate the freq.
			 board->change_counter_frequency (source,speed);
		 }
	 }
	 void dout ()
	 {
		 cerr <<"[SquareWave:board:"<<board->address<<"  source:"<<source<<"]"<<endl;
	 }
};
/*****************************  Switch  *******************************/
/*
		 A switch has an on or off state.
 */
class Switch : public PortBit {
	public:
		unsigned short activestate;
		unsigned short laststate;
		Switch () {
		}
		Switch (DTIO *_board,unsigned int _port,unsigned int _bit,unsigned short as)
		 : PortBit (_board,_port,_bit)
		{
			activestate = as;
			read_state ();
		}
		~Switch () {}

		unsigned short read_state () {
			laststate = state;
			state = get_bit ();
			return (state==activestate);
		}
		int state_changed () {
			return laststate != state;
		}
		int turned_on () {
			return laststate != state && state==activestate;
		}
		int turned_off () {
			return laststate != state && state!=activestate;
		}
	int thestate ()
	{
		 return state==activestate;
	}
	int active ()
	{
		 return state==activestate;
	}
	int passive ()
	{
		 return state!=activestate;
	}
	int on ()
	{
		 return state==activestate;
	}
	int off ()
	{
		 return state!=activestate;
	}};



/******************************  Push switch  **************************/
/*
		push switches allow for multiple button clicks.
 */
class PushSwitch : public Switch {
	public:
		int clicks;
		clock_t stoptime,starttime;
		double elapsed;
		int clickflag;
		int nextstate,lastnextstate,nextnextstate;
		PushSwitch ()
		{
			nextstate = lastnextstate = nextnextstate = 0;
			starttime = clock ();
			clicks = 0;
			clickflag = 0;
			elapsed = 0.0;
			state = 0;
		}
		PushSwitch (DTIO *_board,unsigned int _port,unsigned int _bit, int as)
		 : Switch (_board,_port,_bit,as)
		{
			nextstate = lastnextstate = 0;
			clicks = 0;
			clickflag = 0;
			elapsed = 0.0;
			starttime = clock ();
			read_state ();
		}
		~PushSwitch () {}
		// in order for the below routine to work it has to be called in
		// CLICKTIME millasecond intervals or less.


/*		unsigned short read_state () {
			laststate = state;
			state = get_bit ();
			return (state==activestate);
		}
*/		unsigned short read_state ();

};

/******************************  Control  ********************************/
/*
			 This is an output bit which controls some device.
 */
class Control : public Switch {
	public:

		Control () {
		}
		Control (DTIO *_board,unsigned int _port,unsigned int _bit,unsigned short as)
		 : Switch (_board,_port,_bit,as)
		{
		}
		~Control () {}

		void write_signal (unsigned int signal) {
			if (signal) PortBit::set_bit () ;
			else PortBit::clear_bit ();
			state = signal;
		}
		void set_bit () {
			state = 1;
			//debug4 = state;
			PortBit::set_bit ();
		}
		void clear_bit () {
			state = 0;
			//debug1 = state;
			PortBit::clear_bit ();
		}
		void turn_on () {
			if (activestate) set_bit ();
			else clear_bit ();
		}
		void turn_off () {
			if (activestate) clear_bit ();
			else set_bit ();
		}
};
#endif