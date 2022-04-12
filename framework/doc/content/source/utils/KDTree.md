# KDTree

The [KDTree](https://en.wikipedia.org/wiki/K-d_tree) is a binary tree in which every node is a k-dimensional spatial point. Every non-leaf node implicitly generates a splitting hyperplane that partitions the space into two parts. The left points to the partitioning hyperplane are represented as the left subtree, and similarly, the right points are allocated in the right subtree. The partitioning hyperplane if often 
chosen to be perpendicular to a dimension's axis.

This class implements KDTree by leveraging [nanoflann](https://github.com/jlblancoc/nanoflann). The primary usage is to do the nearest neighbor search. The nearest neighbor search (NN) algorithm aims to find the points in the tree that are nearest to a given input point. This search can be done efficiently by using the tree properties to eliminate large portions of the search space in O(log N) time quickly. More details can be found [here](https://en.wikipedia.org/wiki/K-d_tree).
