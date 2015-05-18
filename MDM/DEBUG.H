#ifndef debughh
#define debughh
class Debug {
	public:
		int numlines;
		HWND hWndLocalList;
		int postlines;
		char postlist[101][256];
		int pause;
		// The debug window handlers.
		Debug () {pause = 0;}
		void initialize (HWND hWnd,long x,long y,long xsize,long ysize);
		void post (char *astring);
		void displayposted ();
		// The debug window handlers.
		void dout (char *astring);
		void dout (char *astring,int anint);
		void dout (char *astring,double adouble);
};
extern Debug debug;
#endif
