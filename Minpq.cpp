#include "Minpq.h"
#include <stdlib.h>
#include <limits.h>

/* initial # of priority queue elements for which to allocate space */
#define MINPQ_INIT_NALLOCD 512

Minpq::Minpq()
{
}

Minpq::~Minpq()
{
	if (pq_array != nullptr)
		release();
}

/** \brief Doubles the size of an array with error checking
 *
 * \param array void**	pointer to an array whose size is to be doubled
 * \param n int			number of elements allocated for \a array
 * \param size int		size in bytes of elements in \a array
 * \return int	eturns the new number of elements allocated for \a array.  If no
 * memory is available, returns 0.
 *
 */
int Minpq::array_double( void **array, int n, int size )
{
	void *tmp = nullptr;

	tmp = realloc( *array, 2 * n * size );
	if ( ! tmp )
	{
		//fprintf( stderr, "Warning: unable to allocate memory in array_double(),"
		//         " %s line %d\n", __FILE__, __LINE__ );
		if ( *array )
			free( *array );
		*array = nullptr;
		return 0;
	}
	*array = tmp;
	return n * 2;
}

/** \brief Decrease a minimizing pq element's key, rearranging the pq if necessary
 *
 * \param i int				index of the element whose key is to be decreased
 * \param key int			new value of element <EM>i</EM>'s key; if greater than current
 * 							key, no action is taken
 * \return void
 *
 */
void Minpq::decrease_pq_node_key( int i, int key )
{
	pq_node tmp;
	int prt = 0;

	if ( key > pq_array[i].key )
		return;

	pq_array[i].key = key;
	prt = parent(i);
	while ( i > 0  &&  pq_array[i].key < pq_array[prt].key )
	{
		tmp = pq_array[prt];
		pq_array[prt] = pq_array[i];
		pq_array[i] = tmp;
		i = prt;
		prt = parent(i);
	}
}

/** \brief Recursively restores correct priority queue order to a minimizing pq array
 *
 * \param i int	index at which to start reordering
 * \param n int	number of elements in \a pq_array
 * \return void
 *
 */
void Minpq::restore_minpq_order( int i, int n )
{
	pq_node tmp;
	int l = 0, r = 0, min = i;

	l = left( i );
	r = right( i );
	if ( l < n )
		if ( pq_array[l].key < pq_array[i].key )
			min = l;
	if ( r < n )
		if ( pq_array[r].key < pq_array[min].key )
			min = r;

	if ( min != i )
	{
		tmp = pq_array[min];
		pq_array[min] = pq_array[i];
		pq_array[i] = tmp;
		restore_minpq_order( min, n );
	}
}

/** \brief Creates a new minimizing priority queue.
 *
 * \return void
 *
 */
void Minpq::init()
{
	if (pq_array != nullptr)
		release();
	pq_array = (pq_node*)calloc( MINPQ_INIT_NALLOCD, sizeof(pq_node) );
	nallocd = MINPQ_INIT_NALLOCD;
	nreal = 0;
}


/** \brief Inserts an element into a minimizing priority queue.
 *
 * \param data void*	[IN] the data to be inserted
 * \param key int		[IN] the key to be associated with \a data
 * \return int
 *
 */
int Minpq::insert(void *data, int key )
{
	if (pq_array == nullptr)
		return -1;

	/* double array allocation if necessary */
	if ( nallocd == nreal )
	{
		nallocd = array_double( (void **)&pq_array,
		                        nallocd,
		                        sizeof(pq_node) );
		if ( nallocd <= 0 )
		{
			//fprintf( stderr, "Warning: unable to allocate memory, %s, line %d\n",
			//         __FILE__, __LINE__ );
			return 1;
		}
	}

	pq_array[nreal].data = data;
	pq_array[nreal].key = INT_MAX;
	decrease_pq_node_key( nreal, key );
	++nreal;

	return 0;
}

/** \brief Returns the element of a minimizing priority queue with the smallest key
 * 			without removing it from the queue.
 *
 * \return void*	the element of \a min_pq with the smallest key or NULL
 * 					if \a min_pq is empty
 *
 */
void *Minpq::get_min()
{
	if ( nreal < 1 )
	{
		//fprintf( stderr, "Warning: PQ empty, %s line %d\n", __FILE__, __LINE__ );
		return nullptr;
	}
	return pq_array[0].data;
}

/** \brief Removes and returns the element of a minimizing priority queue with the
 * 			smallest key.
 *
 * \return void* the element of \a min_pq with the smallest key of NULL
 * 			if \a min_pq is empty
 *
 */
void *Minpq::extract_min()
{
	void *data = nullptr;

	if ( nreal < 1 )
	{
		//fprintf( stderr, "Warning: PQ empty, %s line %d\n", __FILE__, __LINE__ );
		return nullptr;
	}
	data = pq_array[0].data;
	--nreal;
	pq_array[0] = pq_array[nreal];
	restore_minpq_order( 0, nreal );

	return data;
}

/** \brief De-allocates the memory held by a minimizing priorioty queue
 *
 * \return void
 *
 */
void Minpq::release()
{
	if ( pq_array != nullptr )
	{
		free( pq_array );
		pq_array = nullptr;
		nallocd = 0;
		nreal = 0;
	}
}
