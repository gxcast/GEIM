#include "Kdtree.h"
#include <math.h>
#include <stack>

typedef struct _bbf_data_
{
	double d;
	void *old_data;
} bbf_data;


Kdtree::Kdtree()
{
}

Kdtree::~Kdtree()
{
	release();
}


/** \brief Initializes a kd tree node with a set of features.  The node is not
 *    expanded, and no ordering is imposed on the features.
 *
 * \param features ST_FEATURE*	features an array of image features
 * \param n int					number of features
 * \return kd_node*		Returns an unexpanded kd-tree node.
 *
 */
kd_node *Kdtree::kd_node_init( ST_FEATURE *features, int n )
{
	kd_node *node = nullptr;

	node = (kd_node *)malloc( sizeof(kd_node) );
	memset( node, 0, sizeof(kd_node) );
	node->ki = -1;
	node->kv = 0.0;
	node->leaf = 0;
	node->features = features;
	node->n = n;
	node->kd_left = nullptr;
	node->kd_right = nullptr;

	return node;
}

/** \brief Recursively expands a specified kd tree node into a tree whose leaves
 *    contain one entry each.
 *
 * \param node kd_node*	an unexpanded node in a kd tree
 * \return void
 *
 */
void Kdtree::expand_kd_node_subtree( kd_node *node )
{
	/* base case: leaf node */
	if ( node->n == 1  ||  node->n == 0 )
	{
		node->leaf = 1;
		return;
	}

	assign_part_key( node );
	partition_features( node );

	if ( node->kd_left != nullptr )
		expand_kd_node_subtree( node->kd_left );
	if ( node->kd_right != nullptr )
		expand_kd_node_subtree( node->kd_right );
}

/** \brief Determines the descriptor index at which and the value with which to
 *    partition a kd tree node's features.
 *
 * \param node kd_node*	a kd tree node
 * \return void
 *
 */
void Kdtree::assign_part_key( kd_node *node )
{
	ST_FEATURE *features = nullptr;
	double kv, var_max = 0;
	double *tmp = nullptr;
	int d, n, ki = 0;

	features = node->features;
	n = node->n;
	d = features[0].n;

	/* partition key index is that along which descriptors have most variance */
	for (int j = 0; j < d; j++ )
	{
		double mean = 0.0, var = 0.0;
		for (int i = 0; i < n; i++ )
			mean += features[i].descr[j];
		mean /= n;
		for (int i = 0; i < n; i++ )
		{
			double x = features[i].descr[j] - mean;
			var += x * x;
		}
		var /= n;

		if ( var > var_max )
		{
			ki = j;
			var_max = var;
		}
	}

	/* partition key value is median of descriptor values at ki */
	tmp = (double *)calloc( n, sizeof( double ) );
	for (int i = 0; i < n; i++ )
		tmp[i] = features[i].descr[ki];
	kv = median_select( tmp, n );
	free( tmp );

	node->ki = ki;
	node->kv = kv;
}

/** \brief Finds the median value of an array.  The array's elements are re-ordered
 *    by this function.
 *
 * \param array double*	an array; the order of its elelemts is reordered
 * \param n int			number of elements in array
 * \return double	Returns the median value of array.
 *
 */
double Kdtree::median_select( double *array, int n )
{
	return rank_select( array, n, (n - 1) / 2 );
}

/** \brief Finds the element of a specified rank in an array using the linear time
 *    median-of-medians algorithm by Blum, Floyd, Pratt, Rivest, and Tarjan.
 *    The elements of the array are re-ordered by this function.
 *
 * \param array double*	an array; the order of its elelemts is reordered
 * \param n int			number of elements in array
 * \param r int			the zero-based rank of the element to be selected
 * \return double	Returns the element from array with zero-based rank r.
 *
 */
double Kdtree::rank_select( double *array, int n, int r )
{
	double *tmp, med;
	int gr_5, gr_tot, rem_elts, i, j;

	/* base case */
	if ( n == 1 )
		return array[0];

	/* divide array into groups of 5 and sort them */
	gr_5 = n / 5;
	gr_tot = ceil( n / 5.0 );
	rem_elts = n % 5;
	tmp = array;
	for ( i = 0; i < gr_5; i++ )
	{
		insertion_sort( tmp, 5 );
		tmp += 5;
	}
	insertion_sort( tmp, rem_elts );

	/* recursively find the median of the medians of the groups of 5 */
	tmp = (double *)calloc( gr_tot, sizeof( double ) );
	for ( i = 0, j = 2; i < gr_5; i++, j += 5 )
		tmp[i] = array[j];
	if ( rem_elts )
		tmp[i++] = array[n - 1 - rem_elts / 2];
	med = rank_select( tmp, i, ( i - 1 ) / 2 );
	free( tmp );

	/* partition around median of medians and recursively select if necessary */
	j = partition_array( array, n, med );
	if ( r == j )
		return med;
	else if ( r < j )
		return rank_select( array, j, r );
	else
	{
		array += j + 1;
		return rank_select( array, ( n - j - 1 ), ( r - j - 1 ) );
	}
}

/** \brief Sorts an array in place into increasing order using insertion sort.
 *
 * \param array double*	array an array
 * \param n int			number of elements
 * \return void
 *
 */
void Kdtree::insertion_sort( double *array, int n )
{
	double k;
	int i, j;

	for ( i = 1; i < n; i++ )
	{
		k = array[i];
		j = i - 1;
		while ( j >= 0  &&  array[j] > k )
		{
			array[j + 1] = array[j];
			j -= 1;
		}
		array[j + 1] = k;
	}
}

/** \brief Partitions an array around a specified value.
 *
 * \param array double*	array an array
 * \param n int			number of elements
 * \param pivot double	value around which to partition
 * \return int	Returns index of the pivot after partitioning
 *
 */
int Kdtree::partition_array( double *array, int n, double pivot )
{
	double tmp;
	int p, i, j;

	i = -1;
	for ( j = 0; j < n; j++ )
		if ( array[j] <= pivot )
		{
			tmp = array[++i];
			array[i] = array[j];
			array[j] = tmp;
			if ( array[i] == pivot )
				p = i;
		}
	array[p] = array[i];
	array[i] = pivot;

	return i;
}

/** \brief Partitions the features at a specified kd tree node to create its two
 *    children.
 *
 * \param node kd_node*	a kd tree node whose partition key is set
 * \return void
 *
 */
void Kdtree::partition_features( kd_node *node )
{
	ST_FEATURE *features, tmp;
	double kv;
	int n, ki, p, i, j = -1;

	features = node->features;
	n = node->n;
	ki = node->ki;
	kv = node->kv;
	for ( i = 0; i < n; i++ )
	{
		if ( features[i].descr[ki] <= kv )
		{
			tmp = features[++j];
			features[j] = features[i];
			features[i] = tmp;
			if ( features[j].descr[ki] == kv )
				p = j;
		}
	}
	tmp = features[p];
	features[p] = features[j];
	features[j] = tmp;

	/* if all records fall on same side of partition, make node a leaf */
	if ( j == n - 1 )
	{
		node->leaf = 1;
		return;
	}

	node->kd_left = kd_node_init( features, j + 1 );
	node->kd_right = kd_node_init( features + ( j + 1 ), ( n - j - 1 ) );
}

/** \brief Explores a kd tree from a given node to a leaf.  Branching decisions are
 *    made at each node based on the descriptor of a given feature.  Each node
 *    examined but not explored is put into a priority queue to be explored
 *    later, keyed based on the distance from its partition key value to the
 *    given feature's desctiptor.
 *
 * \param expl kd_node*		root of the subtree to be explored
 * \param feat ST_FEATURE*	feature upon which branching decisions are based
 * \param min_pq Minpq*		a minimizing priority queue into which tree nodes are placed
 *    as described above
 * \return kd_node*	Returns a pointer to the leaf node at which exploration ends or
 *    NULL on error.
 *
 */
kd_node *Kdtree::explore_to_leaf( kd_node *expl, ST_FEATURE *feat, Minpq *min_pq )
{
	kd_node *unexpl = nullptr;
	double kv;
	int ki;

	while ( expl != nullptr && expl->leaf == 0)
	{
		ki = expl->ki;
		kv = expl->kv;

		if ( ki >= feat->n )
		{
			//fprintf( stderr, "Warning: comparing imcompatible descriptors, %s line %d\n", __FILE__, __LINE__ );
			return nullptr;
		}
		if ( feat->descr[ki] <= kv )
		{
			unexpl = expl->kd_right;
			expl = expl->kd_left;
		}
		else
		{
			unexpl = expl->kd_left;
			expl = expl->kd_right;
		}

		if ( min_pq->insert( unexpl, fabs(kv - feat->descr[ki]) ) != 0 )
		{
			//fprintf( stderr, "Warning: unable to insert into PQ, %s, line %d\n",
			//         __FILE__, __LINE__ );
			return nullptr;
		}
	}

	return expl;
}

/** \brief Inserts a feature into the nearest-neighbor array so that the array remains
 *    in order of increasing descriptor distance from the search feature.
 *
 * \param feat ST_FEATURE*	feat feature to be inserted into the array; it's feature_data field
 *    should be a pointer to a bbf_data with d equal to the squared descriptor
 *    distance between feat and the search feature
 * \param nbrs ST_FEATURE**	array of nearest neighbors neighbors
 * \param n int				number of elements already in nbrs and
 * \param k int				maximum number of elements in nbrs
 * \return int	If feat was successfully inserted into nbrs, returns 1; otherwise
 *    returns 0.
 *
 */
int Kdtree::insert_into_nbr_array( ST_FEATURE *feat, ST_FEATURE **nbrs, int n, int k )
{
	bbf_data *fdata, * ndata;
	double dn, df;
	int i, ret = 0;

	if ( n == 0 )
	{
		nbrs[0] = feat;
		return 1;
	}

	/* check at end of array */
	fdata = (bbf_data *)feat->feature_data;
	df = fdata->d;
	ndata = (bbf_data *)nbrs[n - 1]->feature_data;
	dn = ndata->d;
	if ( df >= dn )
	{
		if ( n == k )
		{
			feat->feature_data = fdata->old_data;
			free( fdata );
			return 0;
		}
		nbrs[n] = feat;
		return 1;
	}

	/* find the right place in the array */
	if ( n < k )
	{
		nbrs[n] = nbrs[n - 1];
		ret = 1;
	}
	else
	{
		nbrs[n - 1]->feature_data = ndata->old_data;
		free( ndata );
	}
	i = n - 2;
	while ( i >= 0 )
	{
		ndata = (bbf_data *)nbrs[i]->feature_data;
		dn = ndata->d;
		if ( dn <= df )
			break;
		nbrs[i + 1] = nbrs[i];
		i--;
	}
	i++;
	nbrs[i] = feat;

	return ret;
}


/** \brief A function to build a k-d tree database from keypoints in an array.
 *
 * \param features ST_FEATUIRE*	an array of features; <EM>this function rearranges the order
 *    of the features in this array, so you should take appropriate measures if
 *    you are relying on the order of the features (e.g. call this function
 *    before order is important)</EM>
 * \param n 					int	the number of features in \a features
 * \return bool	Returns the root of a kd tree built from \a features.
 *
 */
bool Kdtree::build( ST_FEATURE *features, int n )
{
	kd_node *root = nullptr;
	if ( features != nullptr ||  n <= 0 )
	{
		//fprintf( stderr, "Warning: kdtree_build(): no features, %s, line %d\n", __FILE__, __LINE__ );
		return false;
	}

	root = kd_node_init( features, n );
	expand_kd_node_subtree( root );

	if (kd_root != nullptr)
		release();
	kd_root = root;

	return true;
}

/** \brief Finds an image feature's approximate k nearest neighbors in a kd tree using
 *	  Best Bin First search.
 *
 * \param feat ST_FEATUIRE*		image feature for whose neighbors to search
 * \param k int					number of neighbors to find
 * \param nbrs ST_FEATUIRE***	pointer to an array in which to store pointers to neighbors
 *    in order of increasing descriptor distance; memory for this array is
 *    allocated by this function and must be freed by the caller using
 *    free(*nbrs)
 * \param max_nn_chks int		search is cut off after examining this many tree entries
 * \return int	Returns the number of neighbors found and stored in \a nbrs, or -1 on error.
 *
 */
int Kdtree::bbf_knn( ST_FEATURE *feat, int k, ST_FEATURE *** nbrs, int max_nn_chks )
{
	Minpq *min_pq = nullptr;
	ST_FEATURE ** _nbrs = nullptr;
	int nn_chks = 0, n = 0;

	if ( nbrs == nullptr || feat == nullptr || kd_root == nullptr )
	{
		//fprintf( stderr, "Warning: NULL pointer error, %s, line %d\n",
		//         __FILE__, __LINE__ );
		return -1;
	}

	_nbrs = (ST_FEATURE **)calloc( k, sizeof(ST_FEATURE *) );
	min_pq = new Minpq();
	min_pq->init();
	min_pq->insert( kd_root, 0 );
	while ( min_pq->size() > 0  &&  nn_chks < max_nn_chks )
	{
		kd_node *expl = (kd_node *)min_pq->extract_min();
		if ( expl == nullptr )
		{
			//fprintf( stderr, "Warning: PQ unexpectedly empty, %s line %d\n",
			//         __FILE__, __LINE__ );
			goto fail;
		}

		expl = explore_to_leaf( expl, feat, min_pq );
		if ( expl == nullptr )
		{
			//fprintf( stderr, "Warning: PQ unexpectedly empty, %s line %d\n",
			//         __FILE__, __LINE__ );
			goto fail;
		}

		for ( int i = 0; i < expl->n; i++ )
		{
			ST_FEATURE *tree_feat = &expl->features[i];
			bbf_data *bbf = (bbf_data *)malloc( sizeof(bbf_data) );
			if ( bbf == nullptr )
			{
				//fprintf( stderr, "Warning: unable to allocate memory,"
				//         " %s line %d\n", __FILE__, __LINE__ );
				goto fail;
			}
			bbf->old_data = tree_feat->feature_data;
			bbf->d = feat_dist_sq(feat, tree_feat);
			tree_feat->feature_data = bbf;
			n += insert_into_nbr_array( tree_feat, _nbrs, n, k );
		}
		nn_chks++;
	}

	min_pq->release();
	delete min_pq;
	for ( int i = 0; i < n; i++ )
	{
		bbf_data *bbf = (bbf_data *)_nbrs[i]->feature_data;
		_nbrs[i]->feature_data = bbf->old_data;
		free( bbf );
	}
	*nbrs = _nbrs;
	return n;

fail:
	min_pq->release();
	delete min_pq;
	for ( int i = 0; i < n; i++ )
	{
		bbf_data *bbf = (bbf_data *)_nbrs[i]->feature_data;
		_nbrs[i]->feature_data = bbf->old_data;
		free( bbf );
	}
	free( _nbrs );
	*nbrs = nullptr;
	return -1;
}

/** \brief De-allocates memory held by a kd tree
 *
 * \return void
 *
 */
void Kdtree::release()
{
	if ( kd_root == nullptr )
		return;

	std::stack<kd_node *> stk;
	stk.push(kd_root);
	while (!stk.empty())	// 前序遍历
	{
		kd_node *prt_node = stk.top();
		stk.pop();
		if (prt_node->kd_right != nullptr)
			stk.push(prt_node->kd_right);
		if (prt_node->kd_left != nullptr)
			stk.push(prt_node->kd_left);
		free(prt_node);
	}
}
