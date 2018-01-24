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

bool ParallelUniqueId::_initialized = false;

#ifdef LIBMESH_HAVE_TBB_API
tbb::concurrent_bounded_queue<unsigned int> ParallelUniqueId::_ids;
#elif !defined(LIBMESH_HAVE_OPENMP) && defined(LIBMESH_HAVE_PTHREAD)
std::queue<unsigned int> ParallelUniqueId::_ids;
Threads::spin_mutex ParallelUniqueId::_pthread_id_mutex;
#endif
