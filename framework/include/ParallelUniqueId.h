#ifndef PARALLELUNIQUEID_H
#define PARALLELUNIQUEID_H

#include "libmesh_common.h"

#include "tbb/concurrent_queue.h"

class ParallelUniqueId
{
public:

  ParallelUniqueId()
    {
      ids.pop(id);
    }

  ~ParallelUniqueId()
    {
      ids.push(id);
    }

  static void initialize()
    {
      for(unsigned int i=0; i<libMesh::n_threads(); ++i)
        ids.push(i);
    }
  
  unsigned int id;

protected:
  static tbb::concurrent_queue<unsigned int> ids;
};

#endif // PARALLELUNIQUEID_H
