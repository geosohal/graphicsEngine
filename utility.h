#ifndef UTILITY_H
#define	UTILITY_H

// utility functions for interpolation and other


//#include <glu.h>


namespace Util
{
	template<typename T, typename T2>
	T Lerp(T const& p1, T const& p2, T2 t)
	{
		return (1-t)*p1 + t*p2 ;
	}

}

#endif