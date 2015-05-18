/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/******************************  encoder.cpp  ***************************/
#define INCL_DOSDATETIME
#include <stdio.h>
#include <math.h>
#include <conio.h>
#include "encoder.h"
#include "debug.h"
extern Debug debug;

extern int debug1,debug2,debug3,debug4;
/***************************  Encoder constructor  **************************/

Encoder::Encoder (double range_, int resolution_,
									DT2819 *board_[2],int counter_[2],double toarcseconds)
{
//	 cerr <<"range and resolution init...:"<<range_<<"  "<<
	//resolution_<<endl;
	 // while (!kbhit ()) ; getch ();
	 range = range_; min = 0; max = range-1;
	 keeppositve = 0;
	 common (resolution_,board_,counter_,toarcseconds);
	 // initialize the encoder.
	 init_encoder (resolution,board,counter);
	 // assume last postion of the telescope.
DosSleep (500);
	 set_to_raw_location (50);}
/*
		 Primary initialization for 32 bit encoders.
 */
Encoder::Encoder (double range_, int resolution_,
									DT2819 *board1,DT2819 *board2,
									int counter1,int counter2,double toarcseconds,
									 int _keeppositive)
{
	int counter_[2] = { counter1,counter2};
	DT2819 *board_[2] = {board1,board2};
	 keeppositve = _keeppositive;
	 range = range_; min = 0; max = range-1;

	 common (resolution_,board_,counter_,toarcseconds);
	 // initialize the encoder.
	 init_encoder (resolution,board,counter);

}
Encoder::Encoder (Encoder &encoder)
{
	 range = encoder.range; min = encoder.min; max = encoder.max;
	 common (encoder.resolution,encoder.board,encoder.counter,
																	encoder.toarcsec);
}
/***************************  init encoder  ******************************/
/*
		An encoder consists of either 1 or 2 counters (16 or 32 bits).  This
	 routine initalizes those counters cascading them if necessary.
 */
void Encoder::init_encoder (int resolution,DT2819 *board[2],int counter[4]) {
	int i,j;
	// assume the board is initalized and in a known state.
//  cerr <<"INIT ENCODERS...........................\n";
	for (i=0;i<2;i++) {
	 if (resolution == 16) { // 16 bit, one counter each direction.
		j=i;
			// the counter timer board uses counters 1-5.
//		  cerr <<"Encoder::init:pa_count_events--res=16\n";
			board[i]->count_events (counter[j],0,counter[j]);
	 }
	 else {      // resolution is 32 bit.
	 // remember a 32 bit counter can be counters 1 cacaded to 2 or,
	 // 2 into 3, 3 into 4 or 4 into 5.


		j=i;
			if (counter[j] == COUNTER1) {
//			 cerr <<"Encoder::init:count_events-counter 2-res=32bits\n";
			 board[i]->count_events (counter[j],COUNTER1,CASCADE12);
				// zero in the second argument is for no gates.
			 //cerr <<"Encoder::init:count_events-counter 1-res=32bits\n";
			 //result = board[i]->count_events (counter[j],0,counter[j]);
			}
			else if (counter[j] == COUNTER2) {      // counter[j] == COUNTER3.
//			 cerr <<"Encoder::init:count_events-counter 4-res=32bits\n";
			 board[i]->count_events (counter[j],COUNTER2,CASCADE23);
			 //cerr <<"Encoder::init:count_events-counter 3-res=32bits\n";
			 //result = board[i]->count_events (counter[j],0,counter[j]);
			}		  else if (counter[j] == COUNTER3) {  // counter[j] == COUNTER3.
//			 cerr <<"Encoder::init:count_events-counter 4-res=32bits\n";
			 board[i]->count_events (counter[j],COUNTER3,CASCADE34);
			 //cerr <<"Encoder::init:count_events-counter 3-res=32bits\n";
			 //result = board[i]->count_events (counter[j],0,counter[j]);
			}
			else if (counter[j] == COUNTER4) {      // counter[j] == COUNTER3.
//			 cerr <<"Encoder::init:count_events-counter 4-res=32bits\n";
			 board[i]->count_events (counter[j],COUNTER4,CASCADE45);
			 //cerr <<"Encoder::init:count_events-counter 3-res=32bits\n";
			 //result = board[i]->count_events (counter[j],0,counter[j]);
			}		  else { // remaining 16 bit registers.
//			 cerr <<"Encoder::init:count_events-others-res=16\n";
			 board[i]->count_events (counter[j],0,counter[j]);
			}
	 }
	}
}
/***************************  common  *******************************/
/*
		 common tasks to all of the encoder constructors.
 */
void Encoder::common (int resolution_,DT2819 *board_[2],
							int counter_[4],double toarcsec_)
{
	 int i;
	 resolution = resolution_;
	 cntrange = pow (2,resolution);
//	 cerr <<"INIT:common:"<<endl;
//	 cerr <<"res:"<<resolution<<"  "<<resolution_<<"  cntrange:"<<
		 //cntrange <<endl;
	 for (i=0;i<2;i++) board[i] = board_[i];
//	 cerr <<"after board init....\n";
	 for (i=0;i<2;i++) counter[i] = counter_[i];
	 fwd = 0;
	 rev = 0;
	 toarcsec = toarcsec_;
	 rawoffset=0.;locationoffset=0.;arcsecoffset=0.;encoderoffset=0.;
	 set_to_zero ();
}
/*************************  set to zero  *******************************/
/*
			This manipulates the hardware at a low level to reset the counter
		value to zero.

			see Table 14 on page 46 of Pacer manual for "PA_BUILD_MODE (32" --
		it means to count repeatedly?  <-- I don't remember why this note is here.
*/void Encoder::set_to_zero ()
{

	int mode,result;
	fwd = 0; rev = 0;
	for (int j=0;j<2;j++) {
	 board[j]->set_to_zero (counter[j]);
	}
}
/*****************************  read counters  **************************/
/*
		Read the encoder's counters without interpreting the values.
 */
void Encoder::read_counters (unsigned long tmp[6])
{
	unsigned long temp[6]={0,0,0,0,0,0};

	if (resolution == 16) {
		//debug1 = counter[0]; debug2 = counter[1];
//		cerr <<"16 bit read. ct1:"<<counter[0]<<" ct2:"<<counter[1]<<"\n";
		board[0]->read_counters (counter[0],counter[0],(unsigned int *)temp);
		tmp[FORWARD_LO] = temp[0]; // put result in the right location.
		board[1]->read_counters (counter[1],counter[1],(unsigned int *)temp);
		tmp[REVERSE_LO] = temp[0]; // put result in the right location.
	}
	else { // 32 bit counters.
		//cerr <<"32 bit read. ct1:"<<counter[0]<<" ct2:"<<(counter[0]+1)<<
						 //"  ct3:"<<
		 //			counter[1]<<"  ct4:"<<(counter[1]+1)<<"\n";
		board[0]->read_counters (counter[0],counter[0],(unsigned int *)temp);
		tmp[FORWARD_LO] = temp[0];
		board[0]->read_counters (counter[0]+1,counter[0]+1,(unsigned int *)temp);
		tmp[FORWARD_HI] = temp[0];
		board[1]->read_counters (counter[1],counter[1],(unsigned int *)temp);
		tmp[REVERSE_LO] = temp[0];
		board[1]->read_counters (counter[1]+1,counter[1]+1,(unsigned int *)temp);
		tmp[REVERSE_HI] = temp[0];
		//cerr <<temp[0]<<"  "<<temp[1]<<"  "<<temp[2]<<"  "<<temp[3]<<"  "<<
				 //temp[4]<<endl;
	}
//	char buf[256];
//	sprintf (buf,"READCOUNTERS:fwd lo:%d  fwd hi:%d  rev lo:%d  rev hi:%d ",
//				tmp[FORWARD_LO],tmp[FORWARD_HI],
//				tmp[REVERSE_LO],tmp[REVERSE_HI]);
//	debug.post (buf);	debug.dout (buf);
//debug1 =  tmp[FORWARD_LO];debug2 =  tmp[FORWARD_HI];
//debug3 =  tmp[FORWARD_HI];
//debug4 =  tmp[REVERSE_HI];
}
/***************************  set counters  ***************************/
/*
		 Set the encoder's counters to a particular value as calculated
	 by a different routine.  Not used anymore because of problems setting
	 the DT2819 board's counters.
 */
void Encoder::set_counters (unsigned long tmp[6])
{
	unsigned short temp[6]={0,0,0,0,0,0};

	if (resolution == 16) {
		temp[0] = (unsigned short) tmp[FORWARD_LO];
		board[0]->set_counters (counter[0],counter[0],temp);
		temp[0] = (unsigned short) tmp[REVERSE_LO];
		board[1]->set_counters (counter[1],counter[1],temp);
/*
		temp[0] = tmp[FORWARD_LO];
		board[0]->arm_counters (counter[0],counter[0],temp);
		temp[0] = tmp[REVERSE_LO];
		board[1]->arm_counters (counter[1],counter[1],temp);
*/
	}	else { // 32 bit counters.

		temp[0] = (unsigned short)tmp[FORWARD_HI];
		board[0]->set_counters (counter[0]+1,counter[0]+1,temp);		temp[0] = tmp[FORWARD_LO];
		board[0]->set_counters (counter[0],counter[0],temp);
		temp[0] = (unsigned short)tmp[REVERSE_HI];
		board[1]->set_counters (counter[1]+1,counter[1]+1,temp);		temp[0] = tmp[REVERSE_LO];
		board[1]->set_counters (counter[1],counter[1],temp);


	}//	char buf[256];
//sprintf (buf,"SETCOUNTERS:fwd lo:%ud  fwd hi:%ud  rev lo:%ud  rev hi:%ud ",
//				tmp[FORWARD_LO],tmp[FORWARD_HI],
//				tmp[REVERSE_LO],tmp[REVERSE_HI]);
//	debug.post (buf);	debug.dout (buf);
	 init_encoder (resolution,board,counter);
}
/*********************************  mod  ********************************/
/*
		A floating point modulo arithmatic routine.
 */
double mod (double val,double div)
{
	double mul = val/div;
	return val-(mul*div);
}
/**************************  calculate location  **************************/
long Encoder::calculate_location ()
{
	// test to see if counter rolled over.
	// this works for devices which can't turn in a complete circle.

	if ((double)fwd-(double)rev < -range/2.0 ||
			(double)fwd-(double)rev > range/2.0) { // rolled over.
		// fact: fwd != rev
		rawlocation  = (double)fwd-((double)rev+range);
	}
	else {
		rawlocation = (double)fwd-(double)rev;
	}
	rawlocation += rawoffset;

//	char buf[100];
	if (keeppositve) {
		if ((long)rawlocation == 0) encoderlocation = (long) rawlocation;
		else if ((long)range == 0) encoderlocation = (long) rawlocation;
		else {
			encoderlocation = (long)rawlocation % (long)range;
			while (encoderlocation < 0) encoderlocation += range;
			while (encoderlocation > range) encoderlocation -= range;
		}
		arcseclocation = encoderlocation*toarcsec;

		location = fabs (((double)arcseclocation/3600.0));
	}
	else {
		if ((long)rawlocation == 0) encoderlocation = (long) rawlocation;
		else if ((long)range == 0) encoderlocation = (long) rawlocation;
		else encoderlocation = (long)rawlocation % (long)range;
		arcseclocation = ((double)encoderlocation*toarcsec);
		location = ((double)arcseclocation/3600.0);
	}
//	 sprintf (buf,"fwd:%d  rev:%d  arcsecs:%d  location:%d  raw loc:%d  %lf",
//fwd,rev,arcseclocation,(long)((double)arcseclocation/3600.0),rawlocation,toarcsec);
//	 debug.post (buf);
//	 debug.dout (buf);
	return encoderlocation;
}
/************************  calculate fwd n rev  ***************************/
/*
			 calculate the values to load the counters with given a location.
	 not used...
 */
int Encoder::calculate_fwd_n_rev (double location)
{
	char buf[100];
//		unsigned long modfwd=fwd %65536,modrev=rev%65536;
	arcseclocation = location;
	rawlocation = arcseclocation/toarcsec;
/*
	if (fabs (rawlocation) < range) {
		if (rawlocation < 0) {
			rev = (unsigned long) fabs (rawlocation-0.5); fwd = 0;
		}
		else {
			fwd  = (unsigned long)(rawlocation+0.5); rev = 0;
		}
*/
/*
		modfwd=fwd %65536;modrev=rev%65536;
		int i=0;
		while ((modfwd >= 40000 || modrev >= 40000) && i < 10000) {
			fwd += 8096;  rev += 8096; i++;
			modfwd=fwd %65536;modrev=rev%65536;
		}
		if (i >= 10000) {
		sprintf  (buf,"error in setting location %ul %ul",fwd,rev);
		debug.post (buf);
		return 0;

	}

	}
	else {
		sprintf  (buf,"error in setting location");
		debug.post (buf);
		return 0;
	}
*/
	sprintf (buf,"arcsec: %lf  raw: %lf  fwd: %ud rev: %ud ",
arcseclocation,rawlocation,fwd,rev);//,modfwd,modrev);

	debug.dout (buf); 	debug.post (buf);

	return 1;
}
/**************************  assign fwd n rev  ****************************/
/*
	After a "read_counters", the values of the counters are in a 5 position
	array.  This procedure takes those values and assigns them to the class's
	fwd & rev variables depending on the resolution of the counter.
 */
void Encoder::assign_fwd_n_rev (unsigned long tmp[6])
{

	switch (resolution) {
	 default:
	 case 16:
		fwd = tmp[FORWARD_LO];
		rev = tmp[REVERSE_LO];
		break;
	 case 32:
		fwd = (unsigned long)tmp[FORWARD_HI];
		rev = (unsigned long)tmp[REVERSE_HI];
	 //	fwd *= 65536;
	 //	rev *= 65536;
		fwd <<= 16; // 65536;
		rev <<= 16; //5536;
		//fwd &= (unsigned long )0xffff0000;
		//rev &= (unsigned long )0xffff0000;
		fwd += (unsigned long)tmp[FORWARD_LO];
		rev += (unsigned long)tmp[REVERSE_LO];

		break;
 }
//	char buf[256];
//	sprintf (buf,"ASS FWD N REV::fwd:%ld rev%ld fwd lo:%u  fwd hi:%u  rev lo:%u  rev hi:%u ",
//				fwd,rev,tmp[FORWARD_LO],tmp[FORWARD_HI],
//				tmp[REVERSE_LO],tmp[REVERSE_HI]);
//	debug.post (buf);	debug.dout (buf);
}
/***************************  fwd n rev to bytes  *************************/
/*
	To set the counters the values of forward and reverse have to be
	placed in a five position array.  This is the opposite of assign_fwd_n_rev.

	Not used....
 */
void Encoder::fwd_n_rev_to_bytes (unsigned long tmp[6])
{
	switch (resolution) {
	 default:
	 case 16:
			tmp[FORWARD_LO] = fwd;
			tmp[REVERSE_LO] = rev;
		break;
	 case 32:
			tmp[FORWARD_HI] = (unsigned short) (fwd>>16 & (unsigned long)0xffff);
			tmp[REVERSE_HI] = (unsigned short) (rev>>16 & (unsigned long)0xffff);
			tmp[FORWARD_LO] = (unsigned short) (fwd & (unsigned long)0xffff);
			tmp[REVERSE_LO] = (unsigned short) (rev & (unsigned long)0xffff);
		break;
 }
/*	char buf[256];
	sprintf (buf,"BYTES OF FWD N REV::fwd:%d rev%d fwd lo:%ud  fwd hi:%ud  rev lo:%ud  rev hi:%ud ",
				fwd,rev,tmp[FORWARD_LO],tmp[FORWARD_HI],
				tmp[REVERSE_LO],tmp[REVERSE_HI]);
	debug.post (buf);	debug.dout (buf);
*/
}
/***********************  read arcsec location  ***************************/
/*
		 Read the encoder and return the location in arcseconds.
 */
double Encoder::read_arcsec_location ()
{
	unsigned long tmp[6];
	read_counters (tmp);
//	debug4 = location;
 //	debug2 = fwd;
 //	debug3 = rev;//	lastfwd = fwd;
//	lastrev = rev;
	assign_fwd_n_rev (tmp);
	calculate_location ();
	return arcseclocation;
}
/*********************  read raw location  *****************************/
/*
			Return the location in encoder counts.
 */
double Encoder::read_raw_location ()
{
	unsigned long tmp[6];
	read_counters (tmp);
//	debug4 = location;
 //	debug2 = fwd;
 //	debug3 = rev;//	lastfwd = fwd;
//	lastrev = rev;
	assign_fwd_n_rev (tmp);
	return calculate_location ();
}
/*********************  set to arcsec location  *************************/
/*
		 set the encoder's counters given a location in arcseconds.

	 Not used anymore...
 */
void Encoder::set_to_arcsec_location (double loc)
{
	unsigned long tmp[6];

	//calculate_fwd_n_rev (loc);

	rawoffset =loc/toarcsec;
 //	locationoffset = loc;
//arcsecoffset = locarcseclocation;
	tmp[0] = 0; tmp[1] = 0; tmp[2] = 0; tmp[3] = 0;
	//set_to_zero ();
	set_counters (tmp);
	fwd = 0; rev = 0;
	calculate_location ();
/*
	fwd_n_rev_to_bytes (tmp);
	set_counters (tmp);
*/
}
/*********************** set to raw location  *****************************/
/*   not used..
 */
void Encoder::set_to_raw_location (double loc)
{
	set_to_arcsec_location (loc*toarcsec);
}
/****************************  read fwd  ********************************/
int Encoder::read_fwd ()
{
	unsigned long tmp[5];
	read_counters (tmp);
	assign_fwd_n_rev (tmp);
	return fwd;

}
/****************************** read rev  ********************************/
int Encoder::read_rev ()
{
	unsigned long tmp[5];
  read_counters (tmp);
  assign_fwd_n_rev (tmp);
  return rev;

}
/*****************************  test routines  ***************************/
/*
			Used to debug the encoder routines....
 */
void Encoder::test_count_up (int upcnt) {
	 for (int i=0;i<upcnt;i++)  if (++fwd > cntrange-1) fwd = 0;

}
void Encoder::test_count_dwn (int dwncnt) {
	 for (int i=0;i<dwncnt;i++) if (++rev > cntrange-1) rev = 0;

}
int Encoder::test_read_location () {
	 return calculate_location ();
	 //	 switch (range) {
	 //	case 0:
	 //	  return (abs (fwd-rev) % (max-min+1));
	 //	default:
	 //	  return (abs (fwd-rev) % (range));
	 //}
  }
/*************************  output routines  *****************************/
  ostream &operator << (iostream &os, Encoder &me) {
	 os <<"[Encoder: min:"<<me.min<<" max:"<<me.max<<"  range:"<<
					me.range<<" res:"<<me.resolution<<"]"<<endl;
	 return os;
  }
void Encoder::dout () {
	cout <<"[Encoder: min:"<<min<<" max:"<<max<<"  range:"<<range<<" res:"<<resolution<<"]"<<endl;
  cout <<"[         fwd:"<<fwd<<" rev:"<<rev<<"  counter range:"<<cntrange<<endl;
}
/************************************  set  *******************************/
/*
 */
void Encoder::set (double range_, int resolution_,
								 DT2819 *board_[2],int counter_[2],double toarcsec_)
{
	// cerr <<"range and resolution set init...:"<<range_<<"  "<<resolution_<<endl;
	 //while (!kbhit ()) ; getch ();
	 range = range_; min = 0; max = range-1;
	 common (resolution_,board_,counter_,toarcsec_);
	 // initialize the encoder.
	 init_encoder (resolution,board,counter);
	 // assume last postion of the telescope.
}

Continuous_Encoder::Continuous_Encoder (double range_, int resolution_,
									 DT2819 *board_[2],int counter_[2],double toarcsec_)
	: Encoder (range_,resolution_,board_,counter_,toarcsec_)
{
	init_alarm (range_);
}
Continuous_Encoder::Continuous_Encoder (double range_, int resolution_,
											 DT2819 *board1_,DT2819 *board2_,int counter1_,
												int counter2_,double toarcsec_)
	: Encoder (range_,resolution_,board1_,board2_,counter1_,counter2_,
								toarcsec_,1)
{
	init_alarm (range_);
}
void Continuous_Encoder::init_alarm (double range_)
{
  int i;
  range = range_;
	int revs = cntrange/range;

  revs = range*revs;
  for (i=0;i<2;i++) {
	 if (resolution == 16) { // 16 bit, one counter each direction.
		 board[i]->write_alarm (counter[i],revs);
	 }
  }
}

