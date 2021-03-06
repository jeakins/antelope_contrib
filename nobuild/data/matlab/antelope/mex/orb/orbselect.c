/* 
 * Antelope Toolbox for Matlab
 *
 * Kent Lindquist
 * Geophysical Institute
 * University of Alaska, Fairbanks
 * 1997
 */

#define USAGE "Error using ==> orbselect\n\n\
Usage: NSOURCES = ORBSELECT ( ORBFD, REGEX )\n"

#include "antelope_mex.h"
#include "mex_orb.h"

void mexFunction ( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
	double	orbfd;
	char	*regex;
	int	nselected;

	if( nlhs > 1 )
	{
		antelope_mexUsageMsgTxt ( USAGE );
		return;
	}

	if( nrhs != 2 )
	{
		antelope_mexUsageMsgTxt ( USAGE );
		return;
	}
	if( ! get_scalar( prhs[0], &orbfd ) )
	{
		antelope_mexUsageMsgTxt ( USAGE );
		mexErrMsgTxt( "orbselect: bad orb file descriptor\n" );
	}
	if( ! mtlb_get_string( prhs[1], &regex ) )
	{
		antelope_mexUsageMsgTxt ( USAGE );
	}

	nselected = mex_orbselect( (int) orbfd, regex );

	plhs[0] = CreateDouble( (double) nselected );
	if( plhs[0] == NULL )
	{
		mexErrMsgTxt( "orbselect: failed to create return value" );
	}
}
