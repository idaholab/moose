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

#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"

namespace Moose::Kokkos
{

class NodalReducer : public ReducerBase, public ::BlockRestrictable, public ::BoundaryRestrictable
{
public:
  static InputParameters validParams();

  NodalReducer(const MooseObject * object);

  /**
   * Copy constructor for parallel dispatch
   */
  NodalReducer(const NodalReducer & object);

  virtual void computeReducer() override;

  /**
   * The parallel computation entry function called by Kokkos
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(ReducerLoop, const ThreadID tid, const Derived & reducer, Real * result) const;

protected:
  /**
   * Flag whether this object is boundary-restricted
   */
  const bool _bnd;
  /**
   * Flag for enable/disabling multiple execute calls on nodes that share block ids
   */
  const bool _unique_node_execute;
};

template <typename Derived>
KOKKOS_FUNCTION void
NodalReducer::operator()(ReducerLoop,
                         const ThreadID tid,
                         const Derived & reducer,
                         Real * result) const
{
  auto node = _bnd ? kokkosBoundaryNodeID(tid) : kokkosBlockNodeID(tid);

  Datum datum(node, kokkosAssembly(), kokkosSystems());

  reducer.template reduce<Derived>(datum, result);
}

} // namespace Moose::Kokkos
