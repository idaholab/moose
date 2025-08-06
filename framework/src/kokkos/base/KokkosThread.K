//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosThread.h"

namespace Moose
{
namespace Kokkos
{

void
Thread::resize(std::vector<dof_id_type> dims)
{
  _size = 1;
  _dim = dims.size();

  for (unsigned int dim = 0; dim < _dim; ++dim)
  {
    _dims[dim] = dims[dim];
    _strides[dim] = dim ? _strides[dim - 1] * dims[dim - 1] : 1;
    _size *= dims[dim];
  }
}

} // namespace Kokkos
} // namespace Moose
