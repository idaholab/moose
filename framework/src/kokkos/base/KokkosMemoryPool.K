//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMemoryPool.h"

namespace Moose::Kokkos
{

MemoryPool::MemoryPool(dof_id_type size, unsigned int ways)
{
  MemSpace::memory_space space;

  _pools.create(ways);

  // clang-format off

  for (auto & pool : _pools)
    new (&pool) ::Kokkos::MemoryPool<MemSpace>(space, size / ways);

  // clang-format on

  _pools.copyToDevice();
}

} // namespace Moose::Kokkos
