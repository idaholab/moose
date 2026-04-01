//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGuard.h"
#include "PerfGraph.h"
#ifdef LIBMESH_HAVE_NVTX_API
#include "nvtx3/nvtx3.hpp"
#endif

PerfGuard::PerfGuard(PerfGraph & graph, const PerfID id) : _graph(graph)
{
  _graph.push(id);
#ifdef LIBMESH_HAVE_NVTX_API
  nvtxRangePushA(moose::internal::getPerfGraphRegistry().sectionInfo(id)._name.c_str());
#endif
}

PerfGuard::~PerfGuard()
{
  _graph.pop();
#ifdef LIBMESH_HAVE_NVTX_API
  nvtxRangePop();
#endif
}
