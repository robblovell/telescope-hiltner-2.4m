/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/****************************  dtboard.h  ****************************/
#ifndef	dtboardhh
#define	dtboardhh
#include <iostream.h>

#include <math.h>

#include <iopl.h>
#include "debug.h"
extern unsigned short whichset [8];
extern unsigned short whichclear [8];
#define DTBOARDERROR		-1
/*************************  DTBoard Class  ***************************/
class DTBoard {
	public:
	 unsigned int address;
	 DTBoard (int _address) {
		address = _address;
	 }
	 ~DTBoard () {}

};
/******************************  DTIO Class  ******************************/
class DTIO : public DTBoard{
	public:
	 DTIO (int _address)
		: DTBoard (_address)
	 {}
	 ~DTIO ()  {}

 	 /* 	Virtual functions in a base class allow a pointer to the base
		class to access the functions of classes which inherit the base
		class.
		*/

	 // these virtuals are defined in DT2819.
	 virtual void clear_bit (unsigned int _port,unsigned int _bit) {};
	 virtual void set_bit (unsigned int _port,unsigned int _bit) {}
	 virtual unsigned short get_bit (unsigned int port,unsigned int bit) {return 0;}
	 virtual void set_frequency (double speed) {}
	 virtual void set_counter_frequency (unsigned int counter,double speed) {}
	 virtual double get_frequency (double speed) { return 0.0;}
	 virtual double get_counter_frequency (unsigned int counter,double speed) {return 0.0;}

	 virtual void change_counter_frequency (unsigned int counter,double speed) {}
};
// these define the register offsets for the DT2819 Copunter Timer board.
#define	 AMCONTROL	(address+1)
#define  AMDATA		(address+0)
#define  MASTERMODE	(address+0)
#define  DT19CONTROL	(address+4)
#define  DT19STATUS	(address+4)
#define  DT19PORT0	(address+2)
#define  DT19PORT1	(address+3)
/***************************  DT2819 Class  *******************************/
/*
	The Dt2819 counter timer board has five 16 bit counters, two i/o ports,
	and a frequency out signal.  The Class below allows manipulation of this
	board's control and data registers and some higher level functionality
	for setting up counting, generating frequencies and read and writing
	the data ports.
 */
class DT2819:  public DTIO {
  public:
	 unsigned char status;
	 unsigned short mastermode;
	 int basefrequency;
    // port0 and port1 can be configured for input or output.
	 DT2819 (unsigned int _address,int port0type,int port1type);
	 ~DT2819 () {}
	 DT2819& operator = (const DT2819 &src) {
		status = src.status;
		mastermode = src.mastermode;
		basefrequency = src.basefrequency;
		address = src.address;
		return *this;
	 }
	 void status_wait ();// poll status and wait for ready state.
	 // board initialization.
	 void init_board () {
		cerr <<"init board:"<<address<<endl;
		// Reset the Dt2819 to known state...
		OUT_PORT (DT19CONTROL,0x00);
		OUT_PORT (AMCONTROL,0xFF);
		status_wait ();

		// Reset counters to 0. value comes from load registers.
		OUT_PORT (AMCONTROL,0x5F);
		status_wait ();
		// binary division,no auto inc,data bus 8 bit,fout disabled,divide by 1,
		// source F5,compare disable...
		mastermode = 0x51F0; //0101 0001 1111 0000  set base freq = 1Mhz and
		write_master_mode (mastermode);
		base_freq (5);
		cerr <<"init board done:"<<address<<endl;
	 }	 // functions dealing with counter registers.
	 int count_events (unsigned int source,unsigned int gate,
							 unsigned int counter);
	 void set_to_zero (unsigned int counter);
	 void set_counters (unsigned int startc,
								unsigned int endc,unsigned short value [5]);	 int read_counters (unsigned int startc, unsigned int endc,
								unsigned int result [5]);
	 void arm_counters (unsigned int startc,
						 unsigned int endc);
	 // functions dealing with i/o ports

	 void enable_for_input (unsigned int port,unsigned int latchEnable);
	 void enable_for_output (unsigned int port );

	 void output_digital_value (unsigned int port, unsigned int bit,
				unsigned int value);

	 void output_digital_value (unsigned int port, unsigned int value)
	 {

		OUT_PORT ((USHORT)port+DT19PORT0,(USHORT)value);
		cerr <<"port:"<<port<<" value:"<<value<<endl;
	 }


	 void set_bit (unsigned int port,unsigned int bit) {
		if (((int)bit >= 0) && (bit <= 7)) {
		  //unsigned int value = input_digital_value (port);
		  //value |= (int)pow (2,bit); // set the bit.
		  output_digital_value (port,bit,1); //value);
		}
		else {
		  cerr <<"DT2819::set_bit:bit out of range:"<<bit<<"  \n";
		}
	 }
	 void clear_bit (unsigned int port,unsigned int bit)
	 {
		if (((int)bit >= 0) && (bit <= 7)) {
		  //unsigned int value = input_digital_value (port);
		  //value &= 0xff-(int)pow(2,bit); // clear the bit.
		  output_digital_value (port,bit,0); //value);
		}
		else {
		  cerr <<"DT2819::clear_bit:bit out of range:"<<bit<<"...\n";
		}
	 }
	 unsigned int input_digital_value (unsigned short port)
	 {
		unsigned int value;
		value = IN_PORT ((USHORT)DT19PORT0+port);
		return value;
	 }
	 unsigned int input_digital_value (unsigned int port, unsigned int bit)	 {
		USHORT tvalue = IN_PORT (port+DT19PORT0);
		tvalue &= 0x00FF;
		//fprintf (stderr,"mask: %x  port value:%x  %x",whichset[bit],tvalue,(tvalue & whichset[bit]));
		tvalue = (tvalue & whichset[bit]) >> (bit);
		//fprintf (stderr,"bit:%u bit value:%x  \n",bit,tvalue);
		return tvalue;
	 }

	 unsigned short get_bit (unsigned int port,unsigned int bit) {
		unsigned short value;
		if (((int)bit >= 0) && (bit <= 7)) {
		  //unsigned int value = input_digital_value (port);
		  //value |= (int)pow (2,bit); // set the bit.
		  return (unsigned short) input_digital_value (port,bit);
		}
		else {
		  cerr <<"DT2819::set_bit:bit out of range:"<<bit<<"  \n";
		}
		return DTBOARDERROR;
	 }
	 // functions which read and write registers.
	 void get_status () {
		 status = IN_PORT (DT19STATUS);
	 }
	 void put_status () {
		 OUT_PORT (DT19STATUS,(USHORT)status);
	 }


	 unsigned short read_amdata () {
		 unsigned long low,high,all;

		 low = (unsigned char) (IN_PORT (AMDATA) & 0x00FF);
		 //fprintf (stderr,"amdata lo:%x ",low);
		 high = (unsigned char) (IN_PORT (AMDATA) & 0x00FF);
		 //fprintf (stderr,"amdata hi:%x ",high);
		 high = high << 8;
		 high &= 0xFF00;
		 //fprintf (stderr,"amdata:%x ",high);
		 status_wait ();
		 all = high | low;
		 //fprintf (stderr,"amdata v:%x ",all);
		 return (unsigned short)all;
  }
  void write_amdata (unsigned short value) {
		// write low byte.
		//cerr <<"DT2819::write_amdata: value:";
		//fprintf (stderr,"  %x  %x  %x \n",value,
		//			 (value & 0x00FF), ((value>>8) & 0x00FF));
		OUT_PORT (AMDATA,((value) & (unsigned short) 0x00FF));
		status_wait ();
		// write high byte.
		OUT_PORT (AMDATA,((value >>8) & (unsigned short)0x00FF));
  }
  unsigned short read_master_mode () {
	  unsigned short temp;
		// 00010111
		OUT_PORT (AMCONTROL,0x17);//write to the master mode on next data write.

		return read_amdata ();
	}
	void write_master_mode (unsigned short value) {
		OUT_PORT (AMCONTROL,0x17);//write to the master mode on next data write.
		//fprintf (stderr,"write mastermode: %x",mastermode);
		write_amdata (value);
	}
	void write_mode (unsigned int counter,unsigned short value) {
		// 00010111
		//fprintf (stderr,"COUNTER MODE:%x ",value);
		select_mode (counter); // write to the counter x mode on next data write.
		write_amdata (value);

		select_mode (counter);
		value = read_amdata ();
	 //	fprintf (stderr,"COUNTER MODE AFTER:%x ",value);
	}
	void write_load (unsigned int counter,unsigned short value) {
		select_load (counter); // write to the counter x mode on next data write.
		write_amdata (value);
	}
	void write_alarm (unsigned int counter,unsigned short value) {
		select_alarm (counter); // write to the counter x mode on next data write.
		write_amdata (value);
	}
	unsigned int read_hold (unsigned int counter )
	{
	 select_hold (counter);
	 return read_amdata ();
	}
	void write_hold (unsigned int counter,unsigned short value) {
		select_hold (counter); // write to the counter x mode on next data write.
		write_amdata (value);
  }
  void select_load (unsigned int counter) {
	 OUT_PORT (AMCONTROL,0x08+counter);  //  000 01 + 001-101
	 status_wait ();
  }
  // select the DT2819's hold register for the next data load.
  void select_hold (unsigned int counter) {
	 OUT_PORT (AMCONTROL,0x18+counter); // 000 11 + 001-101
	 status_wait ();
  }
	void select_alarm (unsigned int counter);
  // select the DT2819's mode register for the next data load.
  void select_mode (unsigned int counter) {
	 OUT_PORT (AMCONTROL,0x00+counter); // 000 00 + 001-101
	 status_wait ();
  }
  void load_and_arm (unsigned int counter)
  {
	 //011 00000<- each bit corresponds to counter
	 OUT_PORT (AMCONTROL,0x60+pow(2,counter-1));
	 status_wait ();
  }
	void load_and_arm_counter (unsigned int counter)
  {
	 //011 00000<- each bit corresponds to counter
	 //fprintf (stderr,"load and arm...%x", 0x60+pow(2,counter1-1)+pow(2,counter2-1));
 //	 OUT_PORT (AMCONTROL,0x60+pow(2,counter1)+pow(2,counter2));
	 // load counter
	 OUT_PORT (AMCONTROL,0x40+pow(2,counter-1));
	 // arm counter
	 OUT_PORT (AMCONTROL,0x20+pow(2,counter-1));
	 status_wait ();
	}
	void load_and_arm_counters (unsigned int counter1,unsigned int counter2)
  {
	 //011 00000<- each bit corresponds to counter
	 //fprintf (stderr,"load and arm...%x", 0x60+pow(2,counter1-1)+pow(2,counter2-1));
 //	 OUT_PORT (AMCONTROL,0x60+pow(2,counter1)+pow(2,counter2));
	 // load counter
	 OUT_PORT (AMCONTROL,0x40+pow(2,counter1-1)+pow(2,counter2-1));
	 // arm counter
	 OUT_PORT (AMCONTROL,0x20+pow(2,counter1-1)+pow(2,counter2-1));
	 status_wait ();
	}
	void load_and_arm_all (unsigned int counter)
  {
	 //011 00000<- each bit corresponds to counter
	 OUT_PORT (AMCONTROL,0x7F);
	 status_wait ();
	}
	void save_count_to_hold (unsigned int counter)
	{
	 //cerr <<"to hold reg:"<<counter<<"power"<<pow(2,(double)counter);
	 //cerr <<" code:"<<(0xA0+(unsigned int)pow(2,counter))<<endl;

	 OUT_PORT (AMCONTROL,(0xA0+pow(2,counter-1))); // 101 xxxxx one hold register.
	 status_wait ();
	 //OUT_PORT (AMCONTROL,(0x20+(unsigned int)pow(2,counter))); // 101 xxxxx one hold register.
	 //status_wait ();
	 //save_counts_to_holds ();
	}
	void save_counts_to_holds ()
	{
	 OUT_PORT (AMCONTROL,0xBF); // 101 11111 all of the registers.
	 status_wait ();
	}
	void disarm_counter (unsigned short counter) {
	 OUT_PORT (AMCONTROL,0xC0+pow(2,counter-1)); // 110 xxxxx
	 status_wait ();
	}
	void arm_counter (unsigned short counter) {
	 OUT_PORT (AMCONTROL,0x20+pow(2,counter-1)); // 001 xxxxx
	 status_wait ();
	}
	void arm_all_counters () {
	 OUT_PORT (AMCONTROL,0x3F); // 001 11111
	 status_wait ();
  }
  void load_counter (unsigned int counter) {
	 OUT_PORT (AMCONTROL,0x40+pow(2,counter-1)); // 010 xxxxx
	 status_wait ();
	}
	void load_all_counters () {
	 OUT_PORT (AMCONTROL,0x5F); // 010 11111
	 status_wait ();
  }


	// functions dealing with frequency out functions.
  void fout_on () {

	  mastermode &= 0xEFFF; //0x1000; bit 12 cleared.
	  write_master_mode (mastermode);
	  OUT_PORT (AMCONTROL,0xE6); // 111 00110  fout on.
  }
  void fout_off () {

	 mastermode &= 0x1000; // bit 12 set.
	 write_master_mode (mastermode);
	 OUT_PORT (AMCONTROL,0xEE); // fout off.  // 11101110
	}
  void base_freq (int freq) {
		if (freq == 5) {
			get_status ();
			status &= 0xBF; // clear bit 6.
			put_status ();
			basefrequency = 5;
		}
		else {
			get_status ();
			status |= 0x60; // set bit 6.
			put_status ();
			basefrequency = 1;
		}
  }
	//divisor is a value between 1 and 16. 16 is coded as 0.
	void set_freq_divisor (unsigned short divisor) {
	  unsigned short divcode;

	  cerr <<"divisor:"<<divisor<<endl;
	  if (divisor == 16) {
		 divcode = 0;
	  }
	  else if (divisor < 16) {
		 divcode = divisor;
		}
/*
		else {
		 cerr <<"DT2819::set_freq_divsior:out of range..."<<divisor<<endl;
		 return;
		}
*/
		// divisor goes into bits 8 to 11.
//	  fprintf (stderr,"divcode:%x  mastermode<-%x\n",divcode,mastermode);

	  mastermode = (mastermode | 0x0F00) & ((divcode << 8) | 0xF0FF);
//	  fprintf (stderr,"becomes<-%x  divcode code: %x  %x\n",
//				  mastermode,(divcode<<8),((divcode <<8) | 0xF0FF));
	  write_master_mode (mastermode);
	}
	// source is between 0 and 4 inclusive.
	void set_freq_source (unsigned short source) {
	  unsigned short sourcecode;
	  //mastermode = read_master_mode ();
	  sourcecode = 0xA+source;  // coded as 0xB-0xF
//	  fprintf (stderr,"source:%x  sourcecode:%x  mastermode:%x\n",source,sourcecode,mastermode);
	  mastermode = (mastermode | 0x00F0) & ((sourcecode << 4) | 0xFF0F);
//	  fprintf (stderr,"mastermode:%x\n",mastermode);
		write_master_mode (mastermode);
	}
	void set_frequency (double speed );
	double get_frequency (double speed );
	void calculate_loadvalue_for_freq (double speed,unsigned int &source_code,
																 double &loadvalue,double &source_freq);

	void setup_counter_frequency_generation (unsigned int counter,unsigned int source);

	void set_counter_frequency (unsigned int counter,double speed);
	double get_counter_frequency (unsigned int counter,double speed);
	void change_counter_frequency (unsigned int counter,double speed);
};
// defines offsets to the DT 2816 Digital to Analog board registers.
#define DT16CONTROL	(address+0)
#define DT16STATUS	(address+0)
#define DT16FREQ     (address+1)
#define DT16REGBASE  (address+2)
/*
			 The DT2816 board is a Digital to Analog board.  The object below
	is used to generate voltages on the output DAC's of between 0 and 20 volts.
	The generate volts proceedure simply sets the specified DAC to the
	specified voltage.
 */
class DT2816: public DTBoard {
  public:
	 //unsigned char status;
	 //unsigned short mastermode;
	 DT2816 (unsigned int _address)
		: DTBoard (_address)
	 {
		 cerr <<"DT BOARD 2816 address:"<<_address<<endl;
		 // initialization.
		 OUT_PORT (DT16CONTROL,0x10); // 00010000
		 OUT_PORT (DT16FREQ,0x00);
	 }
	 ~DT2816 () {}
	 /*
				 The generate volts proceedure simply sets the specified DAC to the
		 specified voltage.
		*/
	 void generate_volts (int dac,unsigned short volts) {
			//clear bit 1 of CSR.
			// adv = (analog voltage - minus full scale) *
				// number of codes / (plus full scale - minus full scale)
			// adv = (volts - 0) * 4096/(10-0).
//char buf [256];sprintf (buf,"Volts:%d",volts);
//debug.post (buf);
			unsigned short ADV = (unsigned short) ((double)volts * 409.6);
			OUT_PORT (DT16CONTROL,0x00);
			//fprintf (stderr,"address:%x   control:%x",address,DT16CONTROL);
			//cerr <<"voltage:"<<volts<<"  ADV:"<<ADV<<" low:"<< (ADV & 0x00FF)<<"  high"<<
			//	 ((ADV >> 8) & 0x000F)<<endl;
			OUT_PORT (DT16REGBASE+dac*2,(ADV & 0x00FF)); // low order byte.
			OUT_PORT (DT16REGBASE+dac*2+1,((ADV >> 8) & 0x000F)); // high order nibble.
	 }
/*
	 void clear_bit (unsigned int _port,unsigned int _bit) {}
	 void set_bit (unsigned int _port,unsigned int _bit){}
	 unsigned short get_bit (unsigned int port,unsigned int bit) {return 0;}
	 void set_frequency (double speed){}
	 //void set_frequency (unsigned short divisor,unsigned short frequency){}
*/
};
#endif
