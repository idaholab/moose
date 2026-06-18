//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearSystemContributionObject.h"
#include "KokkosDatum.h"
#include "KokkosVariable.h"

#include "BlockRestrictable.h"
#include "MooseLinearVariableFV.h"
#include "NonADFunctorInterface.h"

namespace Moose::Kokkos
{

/**
 * Base class for Kokkos linear finite volume kernels that contribute to the linear system
 */
class LinearFVKernel : public LinearSystemContributionObject,
                       public BlockRestrictable,
                       public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  LinearFVKernel(const InputParameters & parameters);
  LinearFVKernel(const LinearFVKernel & object);

  /// Tag dispatch type for the right-hand side computation loop
  struct RightHandSideLoop
  {
  };
  /// Tag dispatch type for the matrix computation loop
  struct MatrixLoop
  {
  };

  /**
   * Compute the right-hand side contributions of this kernel
   */
  virtual void computeRightHandSide() = 0;
  /**
   * Compute the matrix contributions of this kernel
   */
  virtual void computeMatrix() = 0;

protected:
  /// The Kokkos variable this kernel acts on
  Variable _kokkos_var;

  /// Dispatcher for the right-hand side computation loop
  std::unique_ptr<DispatcherBase> _rhs_dispatcher;
  /// Dispatcher for the matrix computation loop
  std::unique_ptr<DispatcherBase> _matrix_dispatcher;
};

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
   * @param kernel The concrete kernel object
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(RightHandSideLoop, const ThreadID tid, const Derived & kernel) const;

  /**
   * Matrix dispatch loop body; accumulates the elemental matrix contribution
   * @param tid The thread ID of the current element
   * @param kernel The concrete kernel object
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
  KOKKOS_ASSERT(_kokkos_var.components() == 1);
  const auto & sys = kokkosSystem(_kokkos_var.sys());
  kernel.accumulateTaggedVector(kernel.template computeRightHandSideContribution<Derived>(datum),
                                sys.getElemLocalDofIndex(elem, 0, _kokkos_var.var()));
}

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVElementalKernel::operator()(MatrixLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto elem = kokkosBlockElementID(tid);
  FVDatum datum(elem, libMesh::invalid_uint, kokkosMesh());
  KOKKOS_ASSERT(_kokkos_var.components() == 1);
  const auto & sys = kokkosSystem(_kokkos_var.sys());
  const auto row = sys.getElemLocalDofIndex(elem, 0, _kokkos_var.var());
  kernel.accumulateTaggedMatrix(kernel.template computeMatrixContribution<Derived>(datum),
                                row,
                                sys.getElemGlobalDofIndex(elem, 0, _kokkos_var.var()));
}

/**
 * Base class for Kokkos linear finite volume kernels that contribute on faces (flux terms). On
 * boundary faces the contribution comes from the boundary face data populated by the boundary
 * conditions acting on this kernel's variable.
 */
class LinearFVFluxKernel : public LinearFVKernel, public FaceArgProducerInterface
{
public:
  static InputParameters validParams();

  LinearFVFluxKernel(const InputParameters & parameters);
  LinearFVFluxKernel(const LinearFVFluxKernel & object);

  virtual void computeRightHandSide() override;
  virtual void computeMatrix() override;
  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override;
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

  KOKKOS_ASSERT(_kokkos_var.components() == 1);
  const auto & sys = kokkosSystem(_kokkos_var.sys());
  kernel.accumulateTaggedVector(kernel.template computeRightHandSideContribution<Derived>(datum),
                                sys.getElemLocalDofIndex(elem, 0, _kokkos_var.var()));
}

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVFluxKernel::operator()(MatrixLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto [elem, side] = kokkosBlockElementSideID(tid);
  FVDatum datum(elem, side, kokkosMesh());
  if (!datum.hasNeighbor() && !_bc_data.matrix_coeff.isAlloc())
    return;

  KOKKOS_ASSERT(_kokkos_var.components() == 1);
  const auto & sys = kokkosSystem(_kokkos_var.sys());
  const auto row = sys.getElemLocalDofIndex(elem, 0, _kokkos_var.var());
  kernel.accumulateTaggedMatrix(kernel.template computeMatrixContribution<Derived>(datum),
                                row,
                                sys.getElemGlobalDofIndex(elem, 0, _kokkos_var.var()));

  if (datum.hasNeighbor())
    kernel.accumulateTaggedMatrix(
        kernel.template computeNeighborMatrixContribution<Derived>(datum),
        row,
        sys.getElemGlobalDofIndex(datum.mesh().getNeighbor(elem, side), 0, _kokkos_var.var()));
}

} // namespace Moose::Kokkos
