//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUHeader.h"
#include "GPUMemoryPool.h"

#include "MooseApp.h"

#include "libmesh/parallel_implementation.h"

std::unique_ptr<Moose::Kokkos::MemoryPool> _pool;

void
MooseApp::queryGPUs()
{
  unsigned int num_gpus = Kokkos::num_devices();

  comm().min(num_gpus);

  _has_gpus = num_gpus > 0;
}

void
MooseApp::allocateKokkosMemoryPool(std::size_t size, unsigned int ways) const
{
  _pool = std::make_unique<Moose::Kokkos::MemoryPool>(size, ways);
}

void
MooseApp::deallocateKokkosMemoryPool()
{
  _pool.reset();
}

const Moose::Kokkos::MemoryPool &
MooseApp::getKokkosMemoryPool() const
{
  return *_pool;
}
