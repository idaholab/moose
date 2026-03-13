//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosUserObject.h"
#include "KokkosNodalReducer.h"

#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TransientInterface.h"
#include "RandomInterface.h"

namespace Moose::Kokkos
{

class NodalUserObject : public UserObject,
                        public NodalReducer,
                        public ::CoupleableMooseVariableDependencyIntermediateInterface,
                        public ::TransientInterface,
                        public ::RandomInterface
{
public:
  static InputParameters validParams();

  NodalUserObject(const InputParameters & parameters);

  /**
   * Copy constructor for parallel dispatch
   */
  NodalUserObject(const NodalUserObject & object);

  virtual void compute() override;

  /**
   * The parallel computation entry function called by Kokkos
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(DefaultLoop, const ThreadID tid, const Derived & object) const;

  /**
   * Shim for hook method that can be leveraged to implement static polymorphism
   */
  template <typename Derived>
  KOKKOS_FUNCTION void executeShim(const Derived & object, Datum & datum) const
  {
    object.execute(datum);
  }

  using NodalReducer::operator();
  using NodalReducer::executeShim;

protected:
  virtual void dispatch();
};

template <typename Derived>
KOKKOS_FUNCTION void
NodalUserObject::operator()(DefaultLoop, const ThreadID tid, const Derived & object) const
{
  auto node = _bnd ? kokkosBoundaryNodeID(tid) : kokkosBlockNodeID(tid);

  Datum datum(node, kokkosAssembly(), kokkosSystems());

  object.executeShim(object, datum);
}

} // namespace Moose::Kokkos
