#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vicNl.h>

/****************************************************************************
  Subroutines developed by Bart Nijssen to estimate the daily temperature
  cycle from maximum and minimum daily temperature measurements.  Modified
  June 23, 1998 by Keith Cherkauer to be run within the VIC-NL model.
  ***************************************************************************/

/****************************************************************************/
/*				    hermite                                 */
/****************************************************************************/
/* calculate the coefficients for the Hermite polynomials */
void hermite(int n, 
	     double *x, 
	     double *yc1, 
	     double *yc2, 
	     double *yc3, 
	     double *yc4)
{
  int i;
  double dx;
  double divdf1;
  double divdf3;
  
  for (i = 0; i < n-1; i++) {
    dx = x[i+1] - x[i];
    divdf1 = (yc1[i+1] - yc1[i])/dx;
    divdf3 = yc2[i] + yc2[i+1] - 2 * divdf1;
    yc3[i] = (divdf1 - yc2[i] - divdf3)/dx;
    yc4[i] = divdf3/(dx*dx);
  }
}

/**************************************************************************/
/*				    hermint                               */
/**************************************************************************/
/* use the Hermite polynomials, to find the interpolation function value at 
   xbar */
double hermint(double xbar, int n, double *x, double *yc1, double *yc2, 
	       double *yc3, double *yc4)
{
  int klo,khi,k;
  double dx;
  double result;

  klo=0;
  khi=n-1;
  while (khi-klo > 1) {
    k=(khi+klo) >> 1;
    if (x[k] > xbar) khi=k;
    else klo=k;
  }

  dx = xbar - x[klo];
  result = yc1[klo] + dx * (yc2[klo] + dx * (yc3[klo] + dx * yc4[klo]));
  return result;
}

/****************************************************************************/
/*				    HourlyT                                 */
/****************************************************************************/
void HourlyT(int Dt, int *TmaxHour, double *Tmax, 
	     int *TminHour, double *Tmin, double *Tair)
{
  double *x;
  double *Tyc1;
  double *yc2;
  double *yc3;
  double *yc4;
  int i;
  int j;
  int n;
  int hour;
  int nHours;

  nHours = HOURSPERDAY;
  n = 6;
  x = (double *) calloc(n, sizeof(double));
  Tyc1  = (double *) calloc(n, sizeof(double));
  yc2   = (double *) calloc(n, sizeof(double));
  yc3   = (double *) calloc(n, sizeof(double));
  yc4   = (double *) calloc(n, sizeof(double));

  /* First fill the x vector with the times for Tmin and Tmax, and fill the 
     Tyc1 with the corresponding temperature and humidity values */
  for (i = 0, j = 0, hour = 0.5 * Dt; i < 3; i++, hour += HOURSPERDAY) {
    if (TminHour[i] < TmaxHour[i]) {
      x[j]       = TminHour[i] + hour;
      Tyc1[j++]  = Tmin[i];
      x[j]       = TmaxHour[i] + hour;
      Tyc1[j++]  = Tmax[i];
    }
    else {
      x[j]       = TmaxHour[i] + hour;
      Tyc1[j++]  = Tmax[i];
      x[j]       = TminHour[i] + hour;
      Tyc1[j++]  = Tmin[i];
    }




  }

  /* we want to preserve maxima and minima, so we require that the first 
     derivative at these points is zero */
  for (i = 0; i < n; i++)
    yc2[i] = 0.;

  /* calculate the coefficients for the splines for the temperature */
  hermite(n, x, Tyc1, yc2, yc3, yc4);

  /* interpolate the temperatures */
  for (i = 0, hour = 0.5*Dt+HOURSPERDAY; i < nHours; i++, hour += Dt) {
    Tair[i] = hermint(hour, n, x, Tyc1, yc2, yc3, yc4);
/*****
    if(Tair[i]<Tmin[1]) 
      fprintf(stderr,"WARNING: Estimated air temperature less than daily minimum in hour %i\n",hour-HOURSPERDAY);
    if(Tair[i]>Tmax[1]) 
      fprintf(stderr,"WARNING: Estimated air temperature greater than daily maximum in hour %i\n",hour-HOURSPERDAY);
*****/
  }

  free(x);
  free(Tyc1);
  free(yc2);
  free(yc3);
  free(yc4);
}



/*****double calc_air_temperature(double *tmax, double *tmin, int hour) {*****/
/**********************************************************************
  calc_air_temperature.c	Keith Cherkauer		March 7, 1998

  This subroutine is based on equations from the NWS snow melt model,
  which estimate air temperature based on minimum and maximum daily
  air temperatures for the 6th, 12th, 18th, and 24th hours of the day.

  Modified:
  6/12/98  

**********************************************************************/

