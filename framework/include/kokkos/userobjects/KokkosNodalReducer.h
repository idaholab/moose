//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosReducerBase.h"
#include "KokkosNodalUserObject.h"

namespace Moose::Kokkos
{

class NodalReducer : public ReducerBase, public NodalUserObject
{
public:
  static InputParameters validParams();

  NodalReducer(const InputParameters & parameters);

  virtual void compute() override;

  /**
   * The parallel computation entry function called by Kokkos
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(DefaultLoop, const ThreadID tid, const Derived & reducer, Real * result) const;
};

template <typename Derived>
KOKKOS_FUNCTION void
NodalReducer::operator()(DefaultLoop,
                         const ThreadID tid,
                         const Derived & reducer,
                         Real * result) const
{
  auto node = _bnd ? kokkosBoundaryNodeID(tid) : kokkosBlockNodeID(tid);

  Datum datum(node, kokkosAssembly(), kokkosSystems());

  reducer.executeShim(reducer, datum, result);
}

} // namespace Moose::Kokkos
