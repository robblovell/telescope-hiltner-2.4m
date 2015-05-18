/* --------------------------------------------------------------------
						 Hiltner 2.4 Meter Telescope Control System
															by
												 Robb E. Lovell
-------------------------------------------------------------------- */
/*****************************  motor.cpp  **************************/
#include <iostream.h>
#include "motor.h"

ostream &operator << (iostream &os, Motor &me) {
  os <<"[Motor: max velocity:"<<me.max_speed<<" max accel:"<<me.max_acceleration<<"]"<<endl;
  return os;
}
void Motor::dout () {
  cout <<"[Motor: max velocity:"<<max_speed<<" max accel:"<<max_acceleration<<"]"<<endl;
}
