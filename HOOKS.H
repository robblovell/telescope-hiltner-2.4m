#include "skycalc.h"

extern void get_ut_time (struct date_time  &localTime);
extern void set_local_time (struct date_time &localTime);
extern void get_dates_n_times (struct date_time &localtime,struct date_time &univtime,
															 double &juliandate,
															 double &siderealhours,
																double &currentepoch);
extern void mean_place_to_telescope_coordinates (
										double mean_ra,double mean_dec,double mean_epoch,
										double siderealhours,
										double &ha,double &dec,
										int refractflag,int justprecess);
extern void telescope_coordinates_to_mean_place (
											double ha,double dec,
											double siderealhours,
											double epoch,double &mean_ra,double &mean_dec,
											int refractflag,int justprecess);
extern void telescope_coordinates_to_raw_coordinates
												 (double teleha,double teledec,double &harawarcsec,
													double &decrawarcsec);
extern int set_encoders_telescope_coordinates_to_raw_coordinates
																 (double teleha,double teledec,
																	double &harawarcsec,double &decrawarcsec);extern void raw_coordinates_to_telescope_coordinates (double harawarcsec,double decrawarcsec,double &teleha,double &teledec);
extern void set_encoderpositions ();
extern void set_coordinates ();

extern void set_guide_speeds (double speed);
extern void set_set_speeds (double speed);
extern void calculate_airmass (double &airmass,double &targetairmass);
extern void ratecorr_size(double ha,double dec,double  lat,double  elev,
									 double * haratecorr,double * decratecorr);


extern void figure_trail_directions (double &rate,double length,double posangle,
				 double &ra,double &dec,double &rarate,double &decrate);
extern double IH,ID,PDD,NP, MA,ME, CH, PDD2;
extern void read_telescope_model (double &IH,double &ID,double &PDD,double &NP,
											double &MA,double &ME, double &CH, double &PDD2);
extern void save_last_position ();
extern int restore_last_position (double &rapos,double &decpos,
											 double &domepos,double &focuspos,double &instpos);


