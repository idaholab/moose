//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosFunctorWrapperRegistry.h"

namespace Moose::Kokkos
{

FunctorWrapperRegistry &
FunctorWrapperRegistry::getRegistry()
{
  static FunctorWrapperRegistry * registry = nullptr;
  if (!registry)
    registry = new FunctorWrapperRegistry();
  return *registry;
}

} // namespace Moose::Kokkos
