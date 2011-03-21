#include "ParallelUniqueId.h"

#ifdef LIBMESH_HAVE_TBB_API
tbb::concurrent_bounded_queue<unsigned int> ParallelUniqueId::ids;
bool ParallelUniqueId::initialized = false;
#endif
