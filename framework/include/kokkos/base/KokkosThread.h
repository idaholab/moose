//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_KOKKOS_SCOPE
#include "KokkosHeader.h"
#endif

#include "MooseTypes.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The Kokkos thread object that aids in converting the one-dimensional thread index into
 * multi-dimensional thread indices
 */
class Thread
{
#ifdef MOOSE_KOKKOS_SCOPE
public:
  /**
   * Set the thread pool size and dimension
   * @param dims The vector containing the size of each dimension
   */
  void resize(std::vector<dof_id_type> dims);
  /**
   * Get the total thread pool size
   * @returns The total thread pool size
   */
  dof_id_type size() const { return _size; }
  /**
   * Get the multi-dimensional thread index of a dimension given a one-dimensional thread index
   * @param tid The one-dimensional thread index
   * @param dim for which the multi-dimensional thread index is to be returned
   * @returns The multi-dimensional thread index of the dimension
   */
  KOKKOS_FUNCTION dof_id_type operator()(dof_id_type tid, unsigned int dim) const
  {
    KOKKOS_ASSERT(dim < _dim);

    return (tid / _strides[dim]) % _dims[dim];
  }
#endif

protected:
  /**
   * Total thread pool size
   */
  dof_id_type _size = 0;
  /**
   * Thread pool dimension
   */
  unsigned int _dim = 0;
  /**
   * Thread pool size of each dimension
   */
  dof_id_type _dims[10];
  /**
   * Thread pool stride of each dimension
   */
  dof_id_type _strides[10];
};

} // namespace Kokkos
} // namespace Moose
