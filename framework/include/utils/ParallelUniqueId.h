/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PARALLELUNIQUEID_H
#define PARALLELUNIQUEID_H

#include "Moose.h"  // included for namespace usage

#include "libmesh/libmesh_common.h"
#include "libmesh/threads.h"

#ifdef LIBMESH_HAVE_TBB_API
#include "tbb/concurrent_queue.h"
#include "tbb/tbb_thread.h"
#endif

typedef unsigned int THREAD_ID;

class ParallelUniqueId
{
public:

  ParallelUniqueId()
  {
#ifdef LIBMESH_HAVE_TBB_API
    ids.pop(id);
#else
#ifdef LIBMESH_HAVE_PTHREAD
    id = Threads::pthread_unique_id();
#else
    id = 0;
#endif
#endif
  }

  ~ParallelUniqueId()
  {
#ifdef LIBMESH_HAVE_TBB_API
    ids.push(id);
#endif
  }

  /**
   * Must be called by main thread _before_ any threaded computation!
   * Do NOT call _in_ a worker thread!
   */
  static void initialize()
  {
#ifdef LIBMESH_HAVE_TBB_API
    if(!initialized)
    {
      initialized = true;
      for(unsigned int i=0; i<libMesh::n_threads(); ++i)
        ids.push(i);
    }
#endif
  }

  static void reinitialize()
  {
#ifdef LIBMESH_HAVE_TBB_API
    initialized = false;
    ids.clear();
    initialize();
#endif
  }

  THREAD_ID id;

protected:
#ifdef LIBMESH_HAVE_TBB_API
  static tbb::concurrent_bounded_queue<unsigned int> ids;
#endif

  static bool initialized;
};

#endif // PARALLELUNIQUEID_H
