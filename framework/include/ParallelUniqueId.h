#ifndef PARALLELUNIQUEID_H
#define PARALLELUNIQUEID_H

#include "libmesh_common.h"

#ifdef LIBMESH_HAVE_TBB_API
#include "tbb/concurrent_queue.h"
#endif

class ParallelUniqueId
{
public:

  ParallelUniqueId()
    {
      #ifdef LIBMESH_HAVE_TBB_API
      ids.pop(id);
      #else
      id = 0;
      #endif
    }

  ~ParallelUniqueId()
    {
      #ifdef LIBMESH_HAVE_TBB_API
      ids.push(id);
      #endif
    }

  static void initialize()
    {
      #ifdef LIBMESH_HAVE_TBB_API
      for(unsigned int i=0; i<libMesh::n_threads(); ++i)
        ids.push(i);
      #endif
    }
  
  unsigned int id;

protected:
#ifdef LIBMESH_HAVE_TBB_API
  static tbb::concurrent_queue<unsigned int> ids;
#endif
};

#endif // PARALLELUNIQUEID_H
