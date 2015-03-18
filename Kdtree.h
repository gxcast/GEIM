#ifndef KDTREE_H
#define KDTREE_H

#include "GEIMDef.h"
#include "Minpq.h"

/* the maximum number of keypoint NN candidates to check during BBF search */
#define KDTREE_BBF_MAX_NN_CHKS 200

/** a node in a k-d tree */
typedef struct _kd_node_
{
	int ki;					/**< partition key index */
	double kv;				/**< partition key value */
	int leaf;				/**< 1 if node is a leaf, 0 otherwise */
	ST_FEATURE *features;	/**< features at this node */
	int n;					/**< number of features */
	_kd_node_ *kd_left;		/**< left child */
	_kd_node_ *kd_right;	/**< right child */
} kd_node;


class Kdtree
{
public:
	Kdtree();
	virtual ~Kdtree();

	/** \brief A function to build a k-d tree database from keypoints in an array.
	 *
	 * \param features ST_FEATURE*	an array of features; <EM>this function rearranges the order
	 *    of the features in this array, so you should take appropriate measures if
	 *    you are relying on the order of the features (e.g. call this function
	 *    before order is important)</EM>
	 * \param n 					int	the number of features in \a features
	 * \return bool	Returns the root of a kd tree built from \a features.
	 *
	 */
	bool build( ST_FEATURE *features, int n );

	/** \brief Finds an image feature's approximate k nearest neighbors in a kd tree using
	 *	  Best Bin First search.
	 *
	 * \param feat ST_FEATURE*		image feature for whose neighbors to search
	 * \param k int					number of neighbors to find
	 * \param nbrs ST_FEATURE***	pointer to an array in which to store pointers to neighbors
	 *    in order of increasing descriptor distance; memory for this array is
	 *    allocated by this function and must be freed by the caller using
	 *    free(*nbrs)
	 * \param max_nn_chks int		search is cut off after examining this many tree entries
	 * \return int	Returns the number of neighbors found and stored in \a nbrs, or -1 on error.
	 *
	 */
	int bbf_knn( ST_FEATURE *feat, int k, ST_FEATURE *** nbrs, int max_nn_chks );

	/** \brief De-allocates memory held by a kd tree
	 *
	 * \return void
	 *
	 */
	void release();

private:
	/** \brief Initializes a kd tree node with a set of features.  The node is not
	 *    expanded, and no ordering is imposed on the features.
	 *
	 * \param features ST_FEATURE*	features an array of image features
	 * \param n int					number of features
	 * \return kd_node*		Returns an unexpanded kd-tree node.
	 *
	 */
	kd_node *kd_node_init( ST_FEATURE *features, int n );

	/** \brief Recursively expands a specified kd tree node into a tree whose leaves
	 *    contain one entry each.
	 *
	 * \param node kd_node*	an unexpanded node in a kd tree
	 * \return void
	 *
	 */
	void expand_kd_node_subtree( kd_node *node );

	/** \brief Determines the descriptor index at which and the value with which to
	 *    partition a kd tree node's features.
	 *
	 * \param node kd_node*	a kd tree node
	 * \return void
	 *
	 */
	void assign_part_key( kd_node *node);

	/** \brief Finds the median value of an array.  The array's elements are re-ordered
	 *    by this function.
	 *
	 * \param array double*	an array; the order of its elelemts is reordered
	 * \param n int			number of elements in array
	 * \return double	Returns the median value of array.
	 *
	 */
	double median_select( double *array, int n );

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
	double rank_select( double *array, int n, int r );

	/** \brief Sorts an array in place into increasing order using insertion sort.
	 *
	 * \param array double*	array an array
	 * \param n int			number of elements
	 * \return void
	 *
	 */
	void insertion_sort( double *array, int n );

	/** \brief Partitions an array around a specified value.
	 *
	 * \param array double*	array an array
	 * \param n int			number of elements
	 * \param pivot double	value around which to partition
	 * \return int	Returns index of the pivot after partitioning
	 *
	 */
	int partition_array( double *array, int n, double pivot );

	/** \brief Partitions the features at a specified kd tree node to create its two
	 *    children.
	 *
	 * \param node kd_node*	a kd tree node whose partition key is set
	 * \return void
	 *
	 */
	void partition_features( kd_node *node);

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
	kd_node *explore_to_leaf( kd_node *expl, ST_FEATURE *feat, Minpq *min_pq );

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
	int insert_into_nbr_array( ST_FEATURE *feat, ST_FEATURE **nbrs, int n, int k );


	/**< kdtree's root */
	kd_node *kd_root = nullptr;
};

#endif // KDTREE_H
