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
#include "KokkosElementUserObject.h"

namespace Moose::Kokkos
{

class ElementReducer : public ReducerBase, public ElementUserObject
{
public:
  static InputParameters validParams();

  ElementReducer(const InputParameters & parameters);

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
ElementReducer::operator()(DefaultLoop,
                           const ThreadID tid,
                           const Derived & reducer,
                           Real * result) const
{
  auto elem = kokkosBlockElementID(tid);

  Datum datum(elem, libMesh::invalid_uint, kokkosAssembly(), kokkosSystems());

  reducer.executeShim(reducer, datum, result);
}

} // namespace Moose::Kokkos
