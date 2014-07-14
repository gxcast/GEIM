#ifndef MINPQ_H
#define MINPQ_H

/** an element in a minimizing priority queue */
typedef struct _pq_node_
{
	void *data;
	int key;
} pq_node;

class Minpq
{
public:
	Minpq();
	virtual ~Minpq();

	/**< number of elements in pq */
	inline int size()
	{
		return nreal;
	}

	/** \brief Creates a new minimizing priority queue.
	 *
	 * \return void
	 *
	 */
	void init();

	/** \brief Inserts an element into a minimizing priority queue.
	 *
	 * \param data void*	[IN] the data to be inserted
	 * \param key int		[IN] the key to be associated with \a data
	 * \return int
	 *
	 */
	int insert(void *data, int key );

	/** \brief Returns the element of a minimizing priority queue with the smallest key
	 * without removing it from the queue.
	 *
	 * \return void* the element of \a min_pq with the smallest key or NULL
	 * if \a min_pq is empty
	 *
	 */
	void *get_min();
	/** \brief Removes and returns the element of a minimizing priority queue with the
	 * smallest key.
	 *
	 * \return void* the element of \a min_pq with the smallest key of NULL
	 * if \a min_pq is empty
	 *
	 */
	void *extract_min();

	/** \brief De-allocates the memory held by a minimizing priorioty queue
	 *
	 * \return void
	 *
	 */
	void release();

private:
	/**< returns the array index of element i's parent */
	inline int parent( int i )
	{
		return ( i - 1 ) / 2;
	}
	/**< returns the array index of element i's right child */
	inline int right( int i )
	{
		return 2 * i + 2;
	}
	/**< returns the array index of element i's left child */
	inline int left( int i )
	{
		return 2 * i + 1;
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
	int array_double( void** array, int n, int size );

	/** \brief Decrease a minimizing pq element's key, rearranging the pq if necessary
	 *
	 * \param i int						index of the element whose key is to be decreased
	 * \param key int					new value of element <EM>i</EM>'s key; if greater than current
	 * 									key, no action is taken
	 * \return void
	 *
	 */
	void decrease_pq_node_key(int i, int key);

	/** \brief Recursively restores correct priority queue order to a minimizing pq array
	 *
	 * \param i int	index at which to start reordering
	 * \param n int	number of elements in \a pq_array
	 * \return void
	 *
	 */
	void restore_minpq_order( int i, int n);


	/**< array containing priority queue */
	pq_node *pq_array = nullptr;
	/**< number of elements allocated */
	int nallocd = 0;
	/**< number of elements in pq */
	int nreal = 0;
};

#endif // MINPQ_H
