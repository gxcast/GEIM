#include "GEIMDef.h"
#include <cfloat>

/** \brief Calculates the squared Euclidian distance between two feature descriptors.
 *
 * \param f1 ST_FEATURE*	first feature
 * \param f2 ST_FEATURE*	second feature
 * \return double	Returns the squared Euclidian distance between the descriptors of
 *    \a f1 and \a f2.
 *
 */
double feat_dist_sq( ST_FEATURE *f1, ST_FEATURE *f2 )
{
	double diff, dsq = 0;
	double* descr1, * descr2;
	int i, n;

	n = f1->n;
	if( f2->n != n )
		return DBL_MAX;
	descr1 = f1->descr;
	descr2 = f2->descr;

	for( i = 0; i < n; i++ )
	{
		diff = descr1[i] - descr2[i];
		dsq += diff*diff;
	}
	return dsq;
}

