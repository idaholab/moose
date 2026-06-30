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
 * Base class for Kokkos linear finite volume kernels that contribute on faces (flux terms). On
 * boundary faces the contribution comes from the boundary face data populated by the boundary
 * conditions acting on this kernel's variable.
 */
class LinearFVFluxKernel : public LinearFVKernel
{
public:
  static InputParameters validParams();

  LinearFVFluxKernel(const InputParameters & parameters);
  LinearFVFluxKernel(const LinearFVFluxKernel & object);

  virtual void computeRightHandSide() override;
  virtual void computeMatrix() override;
  virtual void initialSetup() override;

  /// Per-boundary-face contributions, populated by the boundary conditions and consumed by the kernel
  struct BoundaryFaceData
  {
    /// Matrix contribution indexed by (side, element)
    Array2D<Real> matrix_coeff;
    /// Right-hand side contribution indexed by (side, element)
    Array2D<Real> rhs_coeff;
  };

  /**
   * Right-hand side dispatch loop body; accumulates the face RHS contribution (interior faces from
   * the kernel, boundary faces from the precomputed boundary face data)
   * @param tid The thread ID of the current element side
   * @param kernel The concrete kernel object
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(RightHandSideLoop, const ThreadID tid, const Derived & kernel) const;

  /**
   * Matrix dispatch loop body; accumulates the face matrix contribution (interior faces from the
   * kernel, boundary faces from the precomputed boundary face data)
   * @param tid The thread ID of the current element side
   * @param kernel The concrete kernel object
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(MatrixLoop, const ThreadID tid, const Derived & kernel) const;

protected:
  /// Boundary face contributions shared with the boundary conditions acting on this kernel's variable
  BoundaryFaceData _bc_data;
};

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVFluxKernel::operator()(RightHandSideLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto [elem, side] = kokkosBlockElementSideID(tid);
  FVDatum datum(elem, side, kokkosMesh());
  if (!datum.hasNeighbor() && !_bc_data.matrix_coeff.isAlloc())
    return;

  KOKKOS_ASSERT(_var.components() == 1);
  const auto & sys = kokkosSystem(_var.sys());
  kernel.accumulateTaggedVector(kernel.template computeRightHandSideContribution<Derived>(datum),
                                sys.getElemLocalDofIndex(elem, 0, _var.var()));
}

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVFluxKernel::operator()(MatrixLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto [elem, side] = kokkosBlockElementSideID(tid);
  FVDatum datum(elem, side, kokkosMesh());
  if (!datum.hasNeighbor() && !_bc_data.matrix_coeff.isAlloc())
    return;

  KOKKOS_ASSERT(_var.components() == 1);
  const auto & sys = kokkosSystem(_var.sys());
  const auto var_num = _var.var();
  const auto row = sys.getElemLocalDofIndex(elem, 0, var_num);
  kernel.accumulateTaggedMatrix(kernel.template computeMatrixContribution<Derived>(datum),
                                row,
                                sys.getElemGlobalDofIndex(elem, 0, var_num));

  if (datum.hasNeighbor())
    kernel.accumulateTaggedMatrix(
        kernel.template computeNeighborMatrixContribution<Derived>(datum),
        row,
        sys.getElemGlobalDofIndex(datum.mesh().getNeighbor(elem, side), 0, var_num));
}

} // namespace Moose::Kokkos
