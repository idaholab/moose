//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"
#include "KokkosFunctorWrapper.h"

namespace Moose
{
namespace Kokkos
{

class Functor
{
public:
  Functor(std::shared_ptr<FunctorWrapperHostBase> wrapper);
  Functor(const Functor & functor);
  virtual ~Functor();

  KOKKOS_FUNCTION void hello() const { _wrapper_device->hello(); }

private:
  /**
   * Pointer to the host functor wrapper
   */
  std::shared_ptr<FunctorWrapperHostBase> _wrapper_host;
  /**
   * Pointer to the device functor wrapper
   */
  FunctorWrapperDeviceBase * _wrapper_device = nullptr;
};

template <typename T>
class FunctorBase
{
};

} // namespace Kokkos
} // namespace Moose
