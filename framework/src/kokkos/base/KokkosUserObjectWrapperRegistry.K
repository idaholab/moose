//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosUserObjectWrapperRegistry.h"

namespace Moose::Kokkos
{

UserObjectWrapperRegistry &
UserObjectWrapperRegistry::getRegistry()
{
  static UserObjectWrapperRegistry * registry = nullptr;
  if (!registry)
    registry = new UserObjectWrapperRegistry();
  return *registry;
}

} // namespace Moose::Kokkos
