//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearFVKernel.h"
#include "KokkosDatum.h"

namespace Moose::Kokkos
{

/**
 * Base class for Kokkos linear finite volume kernels that contribute on elements (volumetric terms)
 */
class LinearFVElementalKernel : public LinearFVKernel
{
public:
  static InputParameters validParams();

  LinearFVElementalKernel(const InputParameters & parameters);
  LinearFVElementalKernel(const LinearFVElementalKernel & object);

  virtual void computeRightHandSide() override;
  virtual void computeMatrix() override;

  /**
   * Right-hand side dispatch loop body; accumulates the elemental RHS contribution
   * @param tid The thread ID of the current element
   * @param kernel The derived kernel object whose contribution methods are invoked
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(RightHandSideLoop, const ThreadID tid, const Derived & kernel) const;

  /**
   * Matrix dispatch loop body; accumulates the elemental matrix contribution
   * @param tid The thread ID of the current element
   * @param kernel The derived kernel object whose contribution methods are invoked
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(MatrixLoop, const ThreadID tid, const Derived & kernel) const;
};

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVElementalKernel::operator()(RightHandSideLoop,
                                    const ThreadID tid,
                                    const Derived & kernel) const
{
  const auto elem = kokkosBlockElementID(tid);
  FVDatum datum(elem, libMesh::invalid_uint, kokkosMesh());
  KOKKOS_ASSERT(_var.components() == 1);
  const auto & sys = kokkosSystem(_var.sys());
  kernel.template accumulateTaggedVector<AccumulationMode::NonAtomic>(
      kernel.template computeRightHandSideContribution<Derived>(datum),
      sys.getElemLocalDofIndex(elem, 0, _var.var()));
}

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVElementalKernel::operator()(MatrixLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto elem = kokkosBlockElementID(tid);
  FVDatum datum(elem, libMesh::invalid_uint, kokkosMesh());
  KOKKOS_ASSERT(_var.components() == 1);
  const auto & sys = kokkosSystem(_var.sys());
  const auto var_num = _var.var();
  const auto row = sys.getElemLocalDofIndex(elem, 0, var_num);
  kernel.template accumulateTaggedMatrix<AccumulationMode::NonAtomic>(
      kernel.template computeMatrixContribution<Derived>(datum),
      row,
      sys.getElemGlobalDofIndex(elem, 0, var_num));
}

} // namespace Moose::Kokkos
