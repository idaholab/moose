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

      #ifdef TBB_DEPRECATED
      ids.pop(id);
      #else
      ids.try_pop(id);
      #endif
      
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
