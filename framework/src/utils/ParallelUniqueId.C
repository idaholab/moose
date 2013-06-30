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

#include "ParallelUniqueId.h"

#ifdef LIBMESH_HAVE_TBB_API
tbb::concurrent_bounded_queue<unsigned int> ParallelUniqueId::ids;
#else
#ifdef LIBMESH_HAVE_PTHREAD
std::vector<unsigned int> ParallelUniqueId::ids;
Threads::spin_mutex ParallelUniqueId::mutex;
#endif
#endif

bool ParallelUniqueId::initialized = false;
