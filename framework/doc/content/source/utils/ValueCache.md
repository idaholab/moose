# ValueCache

The `ValueCache<T>` class can cache value of type `T` indexed by n-dimensional vectors. Retrieval is performed using a kd-tree data structure (using nanoflann).

## Persistence

When the object is constructed using the

```C++
ValueCache(const std::string & file_name, std::size_t in_dim, std::size_t max_leaf_size = 10);
```

constructor, the state of the cache is saved to a file upon destruction of the cache object (usually at the end of a simulation), and read in at construction of the cache object. Thus the cached knowledge from a previous simulation will be available in future runs.

!alert warning
It is up to the developer to ensure that the cache state from previous runs is applicable to the following runs when the persistence feature is used!

Note that overloads for `dataLoad` and `dataStore` for type `T` must be implemented. If full persistence is not desired the cache object can still be declared as restartable.
