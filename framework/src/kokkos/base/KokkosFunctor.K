//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunctor.h"

namespace Moose
{
namespace Kokkos
{

Functor::Functor(std::shared_ptr<FunctorWrapperHostBase> wrapper) : _wrapper_host(wrapper)
{
  // Allocate device wrapper
  _wrapper_device = _wrapper_host->allocate();
}

Functor::Functor(const Functor & functor)
  : _wrapper_host(functor._wrapper_host), _wrapper_device(functor._wrapper_device)
{
  // Copy functor to device
  _wrapper_host->copyFunctor();
}

Functor::~Functor()
{
  // Free device wrapper
  if (_wrapper_host.use_count() == 1)
    ::Kokkos::kokkos_free<ExecSpace::memory_space>(_wrapper_device);

  // Free host copy of functor
  _wrapper_host->freeFunctor();
}

} // namespace Kokkos
} // namespace Moose
