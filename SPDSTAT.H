#ifndef speedstatushh
#define speedstatushh

#include <stdio.h>
#include <string.h>

class SpeedStatus {
	char hold [100],exchange[100], *message;
	int flopper;

	int _slew,_track,_set,_guide,_fast,_trail;	public:
		SpeedStatus () {
			_slew = 0; _track = 0; _set = 0; _guide = 0; _fast = 0; _trail = 0;
			flopper = 0;
		}
		~SpeedStatus () {

		}
		void slewon () {_slew++;}
		void slewoff () {_slew--;}
		void seton () {_set++;}
		void setoff () {_set--;}
		void guideon () {_guide++;}
		void guideoff () {_guide--;}
		void faston () {_fast++;}
		void fastoff () {_fast--;}
		void trackon () {_track=1;}
		void trackoff () {_track=0;}
		void trailon () {_trail=1;}
		void trailoff () {_trail=0;}
		void update_message (char ***status);

};

extern SpeedStatus rastatus,decstatus;
#endif