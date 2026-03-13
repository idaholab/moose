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

#include "BoundaryRestrictableRequired.h"

namespace Moose::Kokkos
{

class SideReducer : public ReducerBase, public ::BoundaryRestrictableRequired
{
public:
  static InputParameters validParams();

  SideReducer(const MooseObject * moose_object);

  /**
   * Copy constructor for parallel dispatch
   */
  SideReducer(const SideReducer & object);

  virtual void reduce() override;

  /**
   * The parallel computation entry function called by Kokkos
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(ReducerLoop, const ThreadID tid, const Derived & reducer, Real * result) const;
};

template <typename Derived>
KOKKOS_FUNCTION void
SideReducer::operator()(ReducerLoop,
                        const ThreadID tid,
                        const Derived & reducer,
                        Real * result) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(tid);

  Datum datum(elem, side, kokkosAssembly(), kokkosSystems());

  reducer.executeShim(reducer, datum, result);
}

} // namespace Moose::Kokkos
