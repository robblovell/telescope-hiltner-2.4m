/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/*******************************  dtboard.cpp  *****************************/
#include	<iostream.h>
#include "dtboard.h"
#include "hiltner.h"


double frequencies1[5*16]=
{
 10000000.0,500000.0,333333.0,250000.0,200000.0,166667.0,142857.0,
 125000.0,111111.0,100000.0,90909.0,83333.0,76923.0,71429.0,66667.0,62500.0,
 62500.0,31250.0,20833.0,15625.0,12500.0,10417.0,8929.0,7813.0,6944.0,
 6250.0,5682.0,5208.0,4808.0,4464.0,4167.0,3906.0,
 3906.0,1953.0,1302.0,977.0,781.0,651.0,558.0,488.0,434.0,391.0,355.0,
 326.0,300.0,279.0,260.0,244.0,
 244.0,122.0,81.83,61.04,48.83,40.69,34.88,30.52,27.13,24.41,22.19,20.35,
 18.78,17.44,16.28,15.26,
 15.26,7.63,5.09,3.81,3.05,2.54,2.18,1.91,1.70,1.53,1.39,1.27,1.17,1.09,
 1.02,1.01}
;
double frequencies5[5*16]=
{
	5000000.0,2500000.0,1666667.0,1250000.0,1000000.0,833333.0,714286.0,
	625000.0,555556.0,500000.0,454545.0,416667.0,384615.0,357143.0,333333.0,
	312500.0,
	312500.0,156250.0,104167.0,78125.0,62500.0,52083.0,44643.0,39063.0,
	34722.0,31250.0,28409.0,26042.0,24038.0,22321.0,20833.0,19531.0,
	19531.0,9766.0,6510.0,4883.0,3906.0,3255.0,2790.0,2441.0,2170.0,1953.0,
	1776.0,1628.0,1502.0,1395.0,1302.0,1221.0,
	1221.0,610.0,407.0,305.0,244.0,203.0,174.0,153.0,136.0,122.0,111.0,102.0,
	93.90,87.19,81.38,76.29,
	76.29,38.15,25.43,19.07,15.26,12.72,10.90,9.54,8.48,7.63,6.94,6.36,5.87,
	5.45,5.09,4.77}
;
unsigned short whichclear [8] = {0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F };
unsigned short whichset [8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };
/************************  DT2819 constructors  ***********************/
DT2819::DT2819 (unsigned int _address,int port0type,int port1type)
		: DTIO (_address)
{
	  init_board ();
		// configure the counter timer board.
	  switch (port0type) {
		 case 0: enable_for_output (0); break;
		 case 1: enable_for_input (0,0); break;
	  }
	  switch (port1type) {
		 case 0: enable_for_output (1); break;
		 case 1: enable_for_input (1,0); break;
	  }
}
/******************************  status wait  ****************************/
// wait for dt2819 registers to settle.
void DT2819::status_wait ()
{
 		while (((status = IN_PORT (DT19STATUS))>>7) == 1);
	 //	  cerr <<"wait for dt2819 init\n";
}
/***********************  enable for input  *****************************/
/*
			Configure a port for input.
 */
void DT2819::enable_for_input (unsigned int port,unsigned int /*latchEnable*/) {
		status = IN_PORT (DT19STATUS);
		//fprintf (stderr,"board: %x",DT19STATUS);
		//cerr <<"  enable port "<<port<<" for input.  status: "<<(int)status<<endl;
		switch (port) {
			case 0:
			 status &= (unsigned short) 0xFB; // clear bit 2
			 break;
			case 1:
			 status &= (unsigned short) 0xF7; // clear bit 3.
			 break;
		 }
		 OUT_PORT (DT19CONTROL,status);
		 status_wait ();
		status = IN_PORT (DT19STATUS);
		//cerr <<"  status: "<<(int)status<<endl;
}
/***********************  enable for output  *****************************/
/*
			Configure a port for output.
 */void DT2819::enable_for_output (unsigned int port ) {
		status = IN_PORT (DT19STATUS);
	 //	cerr <<"enable port "<<port<<" for output\n";
		switch (port) {
			case 0:
			 status |= (unsigned short) 0x04; // set bit 2
			 break;
			case 1:
			 status |= (unsigned short) 0x08; // set bit 3.
			 break;
		 }
		 OUT_PORT (DT19CONTROL,status);
		 status_wait ();
}
/***********************  output digital value  *************************/
/*
			Change a bit on the given output port to a 1 or 0.
 */
void DT2819::output_digital_value (unsigned int port, unsigned int bit,
				unsigned int value)
{
		USHORT tvalue = IN_PORT (port+DT19PORT0);
	 //	fprintf (stderr,"prior value:%x  ",tvalue);
		switch (value) {
			case 0:
				 tvalue = tvalue & whichclear[bit];
				break;
			case 1:
				 tvalue = tvalue |  whichset[bit];
				break;
		}
		OUT_PORT ((USHORT)port+DT19PORT0,(USHORT)tvalue);
	 //	fprintf (stderr,"port: %x  value:%x  portvalue:%x \n",port,value,tvalue);

}
/*****************************  set frequency  *****************************/
/*
			 Set the counter timer board's frequency out signal.
 */
void DT2819::set_frequency (double speed) {
	unsigned short div,source;
	double found;
	int i,j;
	if (speed == 0) {
	 OUT_PORT (AMCONTROL,0xEE); // fout off.  // 11101110
	}
	else {
/*
	 switch (basefrequency) {
		case 1:
*/
		/* search two tables for the appropriate frequency divisor */
/*		found = -100000000;
			for (i=16*5-2;i >= 0;i--) {
			 if (speed < frequencies1[i]) {
				if (fabs (speed-frequencies1[i]) > fabs (speed-frequencies1[i+1])) {
					i++;
				}
				div = i % 16 +1;
				source = (i / 16) +1;					found = frequencies1[i];
				basefrequency = 1;
				break;
			 }
			}
*/
			for (i=16*5-2;i >= 0;i--) {
			 if (speed < frequencies5[i]) {
				if (fabs (speed-frequencies5[i]) > fabs (speed-frequencies5[i+1])) {
					i++;
				}
			 //	if (fabs (speed-frequencies5[i]) < fabs (speed-found)) {
					div = i % 16 +1;
					source = (i / 16) +1;					found = frequencies5[i];
				//	basefrequency = 5;
			 //	}
				break;
			 }
			}
/*
			break;
		case 5:
		 default:

			for (i=16*5-1;i >= 0;i--) {
			 if (speed < frequencies5[i]) {
				div = i % 16 + 1;
				source = (i / 16) + 1;
				break;
			 }
			}
			break;
*/
	//	cerr <<"speed:"<<speed<<"  i:"<<i<<"  base freq:"<<basefrequency<<"div:"<<div<<"  source:"<<source<<endl;
		set_freq_source (source);
		set_freq_divisor (div);
	//	base_freq (basefrequency);
		OUT_PORT (AMCONTROL,0xE6); // 111 00110  fout on.
	}
}
/*****************************  get frequency  *****************************/
/*
			 Get the actual frequency that will be generated by the counter
		timer board.
 */
double DT2819::get_frequency (double speed) {
	unsigned short div,source;
	int i;
	if (speed == 0) {
		return 0.0;
	}
	else {
	 switch (basefrequency) {
		case 1:
			for (i=16*5-1;i >= 0;i--) {
			 if (speed < frequencies1[i]) {
				//div = i % 16 +1;
				//source = (i / 16) +1;
				return frequencies5[i];
			 }
			}
			break;
		case 5:
		 default:
			for (i=16*5-1;i >= 0;i--) {
			 if (speed < frequencies5[i]) {
				//div = i % 16 + 1;
				//source = (i / 16) + 1;
				return frequencies5[i];
			 }
			}
			break;

		}
	}
	return 0.0;
}/******************  setup counter frequency generation  *******************/
/*
			Use a counter to generate a square wave frequency.
 */
void DT2819::setup_counter_frequency_generation (unsigned int counter,unsigned int source) {
	// 000 nogate, 0 count on rising edge, count source ?-0xB-0xF,
	// 0 no hardware triggers, 1 reload from load reg, 1 count repeatedly,
	// ?0- binary (1-BCD), 0 count down, 010 toggle output.

	// the output of the counter will pulse a signal when its load register
	// is written a value and the counter is loaded and armed.
	unsigned int code = 0x0022 | ((source<<8)&0x0f00);

	write_mode (counter,code);

}
/*******************  calculate loadvalue for frequency  *******************/
	// calculate which source and a load value based upon the speed desired
	// in pulses per second.

	/*
			The source can be one of five internal oscillators:
				F1 (code B) 5000000  == 5 MHz
				F2 (code C) 500000 Hz
				F3 (code D) 50000 Hz
				F4 (code E) 5000 Hz
				F5 (code F) 500 Hz
			From page 69 of the DT2819 user manual comes the following formula:

				X=(source/(frequency wanted * 2). i.e. x=source/(speed*2).

			Search for a frequency source by finding the one that doesn't
			overflow the counter.  Start with the one that most likely will
			overflow and work to the least likely...
	 */

void DT2819::calculate_loadvalue_for_freq (double speed,unsigned int &source_code,double &loadvalue,double &source_freq)
{

	source_code = 0x0B;
	source_freq = 5000000.0;
	loadvalue = source_freq/(speed*2.0);
	// underflow of the counter, set to maximum speed.
	if (loadvalue < 1.0) {
		loadvalue = 1.0;
	}
	// check for overflow of the counter, if there is an overflow,
	// try a slower frequency source.
	while (loadvalue >= 65536 && source_code < 0x10) { // greator than range of counter, use a different source.
		source_code++;
		source_freq/=10.0;
		loadvalue = source_freq/(speed*2.0);
	}
	if (source_code >= 0x10) {
	//cerr <<"error condition setting to slowest speed...\n";
		source_code-- ;
		loadvalue = 65536-1;
	}
}
/**********************  set counter frequency  **************************/
/*
			Use a counter to generate a square wave frequency.
 */
void DT2819::set_counter_frequency (unsigned int counter,double speed)
{
	unsigned int source;
	double loadvalue;
	double source_freq;
	if (speed <= 0.000000001) {
		// 000 nogate, 0 count on rising edge, count source "counter # in",
		// 0 no hardware triggers, 1 reload from load reg, 1 count repeatedly,
		// ?0- binary (1-BCD), 0 count down, 010 toggle output.

		// the output of the counter will pulse a signal when its load register
		// is written a value and the counter is loaded and armed.
		unsigned int code = 0x0064;// + (counter<<8);
		write_mode (counter,code);
		disarm_counter (counter);
//char buf[256];
//sprintf (buf,"STOP!! cntr:%d spd:%lf code:%x",counter,speed,code);
//debug.post (buf);
	}
	else {
		calculate_loadvalue_for_freq (speed,source,loadvalue,source_freq);
	unsigned int code = 0x0022 | ((source<<8)&0x0f00);
//char buf3[256];
//sprintf (buf3,"counter:%d source:%d code:%d",counter,source,code);
//debug.post (buf3);
	write_mode (counter,code);
		setup_counter_frequency_generation (counter,source);
		write_load (counter,(unsigned int) loadvalue);
		load_and_arm (counter);
//char buf2[256];
//sprintf (buf2,"code %x cntr:%d spd:%lf src:%d lv:%lf freq:%lf",code,counter,speed,source,loadvalue,source_freq);
//debug.post (buf2);
	}
}
/**********************  get counter frequency  **************************/
/*
			Use a counter to generate a square wave frequency.
 */
double DT2819::get_counter_frequency (unsigned int ,double speed)
{
	unsigned int source;
	double loadvalue;
	double source_freq;
	if (speed <= 0.000000001) {
		// 000 nogate, 0 count on rising edge, count source "counter # in",
		// 0 no hardware triggers, 1 reload from load reg, 1 count repeatedly,
		// ?0- binary (1-BCD), 0 count down, 010 toggle output.

		// the output of the counter will pulse a signal when its load register
		// is written a value and the counter is loaded and armed.
		unsigned int code = 0x0064;// + (counter<<8);
		//write_mode (counter,code);
		//disarm_counter (counter);
//char buf[256];
//sprintf (buf,"STOP!! cntr:%d spd:%lf code:%x",counter,speed,code);
//debug.post (buf);
		 return 0.0;
	}
	else {
		calculate_loadvalue_for_freq (speed,source,loadvalue,source_freq);
	//unsigned int code = 0x0022 | ((source<<8)&0x0f00);
//char buf3[256];
//sprintf (buf3,"counter:%d source:%d code:%d",counter,source,code);
//debug.post (buf3);
	//write_mode (counter,code);
	 //	setup_counter_frequency_generation (counter,source);
		//write_load (counter,(unsigned int) loadvalue);
		//load_and_arm (counter);
//char buf2[256];
//sprintf (buf2,"code %x cntr:%d spd:%lf src:%d lv:%lf freq:%lf",code,counter,speed,source,loadvalue,source_freq);
//debug.post (buf2);
		return speed;
	}
}/*********************  change counter frequency  *********************/
/*
			Change a counter's square wave frequency.
 */
void DT2819::change_counter_frequency (unsigned int counter,double speed)
{
	unsigned int source;
	double loadvalue;
	double source_freq;
	if (speed <= 0.0) {
		// 000 nogate, 0 count on rising edge, count source "counter # in",
		// 0 no hardware triggers, 1 reload from load reg, 1 count repeatedly,
		// ?0- binary (1-BCD), 0 count down, 010 toggle output.

		// the output of the counter will pulse a signal when its load register
		// is written a value and the counter is loaded and armed.
		unsigned int code = 0x0062 + (counter<<8);

		write_mode (counter,code);

	}
	else {
		calculate_loadvalue_for_freq (speed,source,loadvalue,source_freq);
//char buf[256];
//sprintf (buf,"cntr:%d spd:%lf src:%d lv:%lf freq:%lf",counter,speed,source,loadvalue,source_freq);
//debug.post (buf);
		setup_counter_frequency_generation (counter,source);
		write_load (counter,(unsigned int) loadvalue);
		load_and_arm (counter);
	}
}
/**************************  count events   ******************************/
/*
			setup counters in the given confiuration to count pulses.
		Configurations include single counters as well as two counters used
		together in a cascade.
 */
int DT2819::count_events (unsigned int source,unsigned int gate,
									unsigned int counter)
{
	if (gate == 0 && source < 6 || 1) { // always taken in the MDM program...
	 // counter is 1,2,3,4,5,12,23,34,45,51
	 // source is 0-5 0 is counter-1, 1-5 is source 1-source 5.
	 // in MDM's case, source is defined according to the counter #.  Each
	 // counter always counts from it's equivalent numbered source.
//cerr <<"counter:#"<<counter<<endl;
	 switch (counter) {
		case 1: case 2: case 3: case 4: case 5:
			// 000 nogate, 0 count on rise edge, xxxx source, 0 no hardware trigger,
			// 0 reload from load register, 1 count repeats, 0 binary,
			// 1 count up, 000 no output.  0000xxxx 0010 1010
			// 0x0X28 0x0XA8
//cerr <<"set up counter #"<<0x1028<<(counter<<8)<<" ..........\n";
//fprintf (stderr,"MODE::::::::::::::::%x",0x1028+(counter<<8));
			write_mode (counter,0x1028+(counter<<8));
			load_and_arm (counter);
			break;
		case 12:
			// counter 1 output is active, counter 2 source is counter 1 output.
//cerr <<"set up counters 1 and 2 to cascade ..........\n";
			write_mode (1,0x1129);
			write_mode (2,0x1028);
			load_and_arm_counters (1,2);
			break;
		case 23:
//cerr <<"set up counters 2 and 3 to cascade ..........\n";
			write_mode (2,0x1229);
			write_mode (3,0x1028);
			load_and_arm_counters (2,3);
			break;
		case 34:
//cerr <<"set up counters 3 and 4 to cascade ..........\n";
			write_mode (3,0x1329);
			write_mode (4,0x1028);
			load_and_arm_counters (3,4);
			break;
		case 45:
			write_mode (4,0x1429);
			write_mode (5,0x1028);
			load_and_arm_counters (4,5);
			break;
		case 51:
			write_mode (5,0x1529);
			write_mode (1,0x1028);
			load_and_arm_counters (5,1);
			break;
	 }
	}

	return 0;
}
/**************************  set to zero  ********************************/
/*
		 Set a counter to zero.
 */
void DT2819::set_to_zero (unsigned int counter)
{
	// select the load register.
	write_load (counter,0);
	// tell the counter to load the value.
	load_and_arm (counter);
}
/****************************  arm counters  ******************************/
/*
		Activate a set of counters.
 */
void DT2819::arm_counters (unsigned int startc,
						 unsigned int endc)
{
	// put count into hold registers

	for (int i = startc;i<=endc;i++) {
		arm_counter (i);
	}
}
/******************************  set counters  **************************/
/*
			set the given counters to the values given.  This procedure does
	 not work consistently and is not used....
 */
void DT2819::set_counters (unsigned int startc,
						 unsigned int endc,unsigned short value [5])
{
	// put count into hold registers
	int j=0;
	for (int i = startc;i<=endc;i++) {
		write_load (i,value[j++]);
		load_counter (i);
	}
}
/**************************  read counters  *****************************/
/*
		 Read the state of the given counters.
 */
int DT2819::read_counters (unsigned int startc,
						 unsigned int endc,unsigned int result [5])
{
	// put count into hold registers

	cerr <<"DT2819::read_counters: start:"<<startc<<" endc:"<<endc<<endl;
	int j=0;
	for (int i = startc;i<=endc;i++) {
	 save_count_to_hold (i);
	 // read hold register.
	 result[j++] = read_hold (i);
	}
	return 0;
}
/**************************  select alarm  *****************************/
/*
			 turn on the alarm for the given counter.  used in counter based
		frequency generation.
 */
void DT2819::select_alarm (unsigned int counter) {
	 switch (counter) {
		case 1:
			OUT_PORT (AMCONTROL,0x07);  //  000 01 + 001-101
			break;
		case 2:
			OUT_PORT (AMCONTROL,0x0f);  //  000 01 + 001-101
			break;
		default:
			break;
	 }
	 status_wait ();
}