//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelUniqueId.h"

bool ParallelUniqueId::_initialized = false;

#ifdef LIBMESH_HAVE_TBB_API
tbb::concurrent_bounded_queue<unsigned int> ParallelUniqueId::_ids;
#elif !defined(LIBMESH_HAVE_OPENMP) && defined(LIBMESH_HAVE_PTHREAD)
std::queue<unsigned int> ParallelUniqueId::_ids;
Threads::spin_mutex ParallelUniqueId::_pthread_id_mutex;
#endif
