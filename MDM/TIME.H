

#ifndef Timehh
#define Timehh
#include <iostream.h>
#include <string.h>
//#include <times.h>
//#include <unistd.h>

/*
 * Forward class declarations
 */


class Time;
#define _SC_CLK_TCK	0
double sysconf (int ) {
 return 18.0;
}
/*
 * Class declarations
 */
class Time{
  private:
    friend ostream& operator<<(ostream&, Time&);
  protected:
  public:
    int hr,min,sec,fsec;
    long clk_tck;
    int convert12to24 (int hour,char *period)
    {
			if (!strcmp (period,"am")) {
        if (hour == 12) return 0;
        else return hour;
      }
      else {
        if (hour == 12) return 12;
        else return hour + 12;
      }
    }

    // constructors.
    Time () {}
    // set by 24 hour time.
    Time (int h, int m) {min=m; sec=0; hr=h;fsec = 0; }
    Time (int h, int m, int s) {min=m; sec=s; hr=h;fsec = 0; }
    Time (int h, int m, int s,int f) {min=m; sec=s; hr=h; fsec = f; }
    // set by 12 hour time.
    Time (int h, int m, int s, int fs, char *period) {min=m; sec=s; hr=convert12to24(h,period); fsec = fs; }
    Time (int h, int m, int s, char *period) {min=m; sec=s; hr=convert12to24(h,period); fsec = 0; }
    Time (int h, int m, char *period) {min=m; sec=0; hr=convert12to24(h,period); fsec = 0; }

    // set by seconds and fseconds.
    Time (double seconds) 
    {
      hr = (int)((double)seconds/60.0/60.0);
      min = (int)((double)seconds/60.0)-hr*60;
      sec = (int)seconds-min*60-hr*60*60; 
      fsec = (int) (seconds*clk_tck-sec*clk_tck-min*clk_tck*60-hr*clk_tck*60*60); 
    }
    Time (int fseconds) 
    {
			clk_tck = sysconf (_SC_CLK_TCK);
      hr = (int)((double)fseconds/clk_tck/60.0/60.0);
      min = (int)((double)fseconds/clk_tck/60.0)-hr*60;
      sec = (int)((int)fseconds/clk_tck-min*60-hr*60*60); 
      fsec = (int)((int)fseconds-sec*clk_tck-min*clk_tck*60-hr*clk_tck*60*60); 
    }
    // operators
    operator int ();
    int operator==(Time );
    Time operator=(Time );
    Time operator=(int );
    Time operator=(double );
    Time operator+(int);
    Time operator+(Time);
    void adjust_min ();
    void adjust_sec ();
    void adjust_fsec ();
    char *ampm ();

    int seconds (); 
    int fseconds (); 
    int second () {return sec;}
    int minute () {return min;}
    int hour () {return hr;}


    Time set(int, int, int);

    Time set (int h, int m, int s, int fs) {min=m; sec=s; hr=h; fsec = fs; return *this; }
    Time set (int h, int m) {min=m; sec=0; hr=h; return *this; }
    // set by 12 hour time.
    Time set (int h, int m, int s, int fs, char *period) {min=m; sec=s; fsec = fs; hr=convert12to24(h,period); return *this; }
    Time set (int h, int m, int s, char *period) {min=m; sec=s; hr=convert12to24(h,period); return *this; }
    Time set (int h, int m, char *period) {min=m; sec=0; hr=convert12to24(h,period); return *this; }

    void reset () {hr = 0;min=0;sec = 0; fsec = 0;}
};

#endif
