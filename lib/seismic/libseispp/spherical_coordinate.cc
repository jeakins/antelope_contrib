#include <float.h>
#include "seispp.h"
namespace SEISPP
{
using namespace SEISPP;
/* This routine takes a 3-d unit vector, nu, and converts it
to a Spherical_Coordinate structure which is returned.  The 
input coordinates are assume to be standard, right handed
cartesian coordinates in 1,2,3 order */
Spherical_Coordinate unit_vector_to_spherical(double nu[3])
{
	Spherical_Coordinate xsc;

	xsc.radius = 1.0;
	xsc.theta = acos(nu[2]);
	xsc.phi = atan2(nu[1],nu[0]);
	return(xsc);
}
/* Reciprocal of above.  A bit harder as it has to handle singular
case.  Note the double vector of 3 is allocated here and externally
needs to be free using C++ delete NOT free.*/
double *spherical_to_unit_vector(Spherical_Coordinate& scor)
{
	double *nu=new double[3];
	// vertical vector case
	if(fabs(scor.theta)<= DBL_EPSILON) 
	{
		nu[0]=0.0;
		nu[1]=0.0;
		nu[2]=1.0;
	}
	else
	{
		nu[0]=sin(scor.theta)*cos(scor.phi);
		nu[1]=sin(scor.theta)*sin(scor.phi);
		nu[3]=cos(scor.theta);
	}
	return(nu);
}
} // end namespace declaration
