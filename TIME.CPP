


#include <iostream.h>
#include <time.h>
#include "Time_.h"
double sysconf (int ) {
 return 18.0;
}//
Time::operator int ()
{
  return (hr*60*60+min*60+sec);
}
//---------- Time Class Functions ---------------------------------------------
ostream& operator<<(ostream& os, Time& p) 
{ 
  os <<p.hr<<":"<<p.min<<":"<<p.sec<<":"<<p.fsec;
  return os; 
} /* used with cout */
int Time::operator==(Time p)
{
  if (sec == p.sec && min == p.min && hr == p.hr)
    return (1);
  else 
    return (0);
}
Time Time::operator=(Time p)
{
  fsec = p.fsec; sec = p.sec; min = p.min; hr = p.hr;
  return *this;
}
Time Time::operator=(double seconds)
{
      hr = (int)((double)seconds/60.0/60.0);
      min = (int)((double)seconds/60.0)-hr*60;
      sec = (int)seconds-min*60-hr*60*60;
      fsec = (int)(seconds*clk_tck-sec*clk_tck-min*clk_tck*60-hr*clk_tck*60*60);

  return *this;
}
Time Time::operator=(int fseconds)
{
      clk_tck = sysconf (_SC_CLK_TCK);
      hr = (int)((double)fseconds/clk_tck/60.0/60.0);
      min = (int)((double)fseconds/clk_tck/60.0)-hr*60;
      sec = (int)(fseconds/clk_tck-min*60-hr*60*60);
      fsec = (int)(fseconds-sec*clk_tck-min*clk_tck*60-hr*clk_tck*60*60);

  return *this;
}
/*
	Add two Times. 
 */
void Time::adjust_min ()
{
  while (min > 60) {
    hr++;
    min -= 60;
  }
}
void Time::adjust_sec ()
{
  while (sec > 60) {
    min ++;
    adjust_min ();
    sec -= 60;
  }
}
void Time::adjust_fsec ()
{
  while (fsec > clk_tck) {
    sec++;
    adjust_sec ();
    fsec -= (int)clk_tck;
  }
}
Time operator+(Time a, Time b)
{
  Time c;
  c.hr = a.hr + b.hr;
  c.min = a.min + b.min;  
  c.adjust_min ();
  c.sec = a.sec + b.sec;  
  c.adjust_sec ();
  c.fsec = a.fsec + b.fsec;  
  c.adjust_fsec ();
  return (c);
}
Time Time::operator+(int d)
{
  fsec += d;
  adjust_fsec ();
  return (*this);
}
char *am="am";
char *pm="pm";
char *Time::ampm ()
{
  if (hr >= 12) return pm;
  return am;
}

int Time::seconds () { return hr*3600+min*60+sec; }
int Time::fseconds () { return hr*3600*60+min*60*60+sec*60+fsec; }



