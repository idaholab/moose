//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_GPU_SCOPE
#include "GPUHeader.h"
#endif

#include "MooseTypes.h"

class GPUThread
{
private:
  // Total thread pool size
  size_t _size = 0;
  // Thread pool dimensions
  size_t _dims[10];
  // Thread ID divisor of each dimension
  size_t _divisor[10];

#ifdef MOOSE_GPU_SCOPE
public:
  void resize(std::vector<size_t> dims)
  {
    _size = 1;

    for (unsigned int dim = 0; dim < dims.size(); ++dim)
    {
      _dims[dim] = dims[dim];
      _divisor[dim] = dim ? _divisor[dim - 1] * dims[dim - 1] : 1;
      _size *= dims[dim];
    }
  }
  auto size() const { return _size; }

  KOKKOS_FUNCTION auto operator()(size_t tid, unsigned int dim) const
  {
    return (tid / _divisor[dim]) % _dims[dim];
  }
#endif
};
