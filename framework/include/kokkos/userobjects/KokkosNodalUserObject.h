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
#include "KokkosMaterialPropertyValue.h"

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
  virtual unsigned int getUOExecutionOrderWithinGroup() const override final { return 0; }

  /**
   * The parallel computation entry function called by Kokkos
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(DefaultLoop, const ThreadID tid, const Derived & object) const;

  using NodalReducer::operator();

protected:
  virtual ThreadID numUserObjectThreads() const override
  {
    return _bnd ? numKokkosBoundaryNodes() : numKokkosBlockNodes();
  }
};

template <typename Derived>
KOKKOS_FUNCTION void
NodalUserObject::operator()(DefaultLoop, const ThreadID tid, const Derived & object) const
{
  auto node = _bnd ? kokkosBoundaryNodeID(tid) : kokkosBlockNodeID(tid);

  Datum datum(node, kokkosAssembly(), kokkosSystems());

  object.template execute<Derived>(datum);
}

} // namespace Moose::Kokkos
