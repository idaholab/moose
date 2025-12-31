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

namespace Moose
{
namespace Kokkos
{

class UserObject : public ::UserObjectBase,
                   public MeshHolder,
                   public AssemblyHolder,
                   public SystemHolder
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

  /**
   * Shim for hook method that can be leveraged to implement static polymorphism
   */
  template <typename Derived>
  KOKKOS_FUNCTION void executeShim(const Derived & userobject, Datum & datum) const
  {
    userobject.execute(datum);
  }

protected:
  /**
   * Kokkos functor dispatcher
   */
  std::unique_ptr<DispatcherBase> _dispatcher;
};

} // namespace Kokkos
} // namespace Moose
