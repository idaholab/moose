//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunctorRegistry.h"

namespace Moose
{
namespace Kokkos
{

FunctorRegistry &
FunctorRegistry::getRegistry()
{
  static FunctorRegistry * registry = nullptr;
  if (!registry)
    registry = new FunctorRegistry();
  return *registry;
}

} // namespace Kokkos
} // namespace Moose
