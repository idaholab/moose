//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMesh.h"
#include "KokkosAssembly.h"
#include "KokkosSystem.h"
#include "KokkosDispatcher.h"

#include "UserObjectBase.h"

namespace Moose::Kokkos
{

class UserObject : public ::UserObjectBase
{
public:
  static InputParameters validParams();

  UserObject(const InputParameters & params);

  /**
   * Copy constructor for parallel dispatch
   */
  UserObject(const UserObject & object);

  // Unused for Kokkos user objects because all subdomains are computed in parallel
  virtual void subdomainSetup() override final {}

  /**
   * Compute this user object
   */
  virtual void compute() = 0;

  /**
   * Kokkos function tag
   */
  struct DefaultLoop
  {
  };
};

} // namespace Moose::Kokkos
