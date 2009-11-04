#include "ParallelUniqueId.h"

#ifdef LIBMESH_HAVE_TBB_API
tbb::concurrent_queue<unsigned int> ParallelUniqueId::ids;
#endif
