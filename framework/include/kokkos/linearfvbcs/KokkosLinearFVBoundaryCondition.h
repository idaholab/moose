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

#include "BoundaryRestrictableRequired.h"
#include "GeometricSearchInterface.h"
#include "NonADFunctorInterface.h"
#include "FaceArgInterface.h"

#include "KokkosLinearFVKernel.h"

namespace Moose::Kokkos
{

/**
 * Base class for Kokkos linear finite volume boundary conditions. It populates the
 * per-boundary-face matrix and right-hand side data that the linear FV flux kernels consume during
 * assembly.
 */
class LinearFVBoundaryCondition : public LinearSystemContributionObject,
                                  public BoundaryRestrictableRequired,
                                  public GeometricSearchInterface,
                                  public NonADFunctorInterface,
                                  public FaceArgProducerInterface
{
public:
  static InputParameters validParams();

  LinearFVBoundaryCondition(const InputParameters & parameters);
  LinearFVBoundaryCondition(const LinearFVBoundaryCondition & object);

  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override;

  /// Tag dispatch type for the right-hand side computation loop
  struct RightHandSideLoop
  {
  };
  /// Tag dispatch type for the matrix computation loop
  struct MatrixLoop
  {
  };

  /**
   * Compute the right-hand side contributions of this boundary condition
   */
  virtual void computeRightHandSide();
  /**
   * Compute the matrix contributions of this boundary condition
   */
  virtual void computeMatrix();
  /**
   * Bind this boundary condition to the owning flux kernel's boundary face data
   * @param data The owning flux kernel's boundary face data to write into
   */
  void initBCData(const LinearFVFluxKernel::BoundaryFaceData & data);

  /**
   * Right-hand side dispatch loop body; writes the per-face RHS contribution into the shared data
   * @param tid The thread ID of the current boundary side
   * @param bc The concrete boundary condition object
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(RightHandSideLoop, const ThreadID tid, const Derived & bc) const;

  /**
   * Matrix dispatch loop body; writes the per-face matrix contribution into the shared data
   * @param tid The thread ID of the current boundary side
   * @param bc The concrete boundary condition object
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(MatrixLoop, const ThreadID tid, const Derived & bc) const;

  /**
   * Get the underlying MOOSE linear finite volume variable
   * @returns The MOOSE variable this boundary condition acts on
   */
  const MooseLinearVariableFV<Real> & mooseVariable() const { return _moose_var; }

  /**
   * Get the Kokkos variable
   * @returns The Kokkos variable this boundary condition acts on
   */
  Variable variable() const { return _var; }

protected:
  /// The MOOSE linear finite volume variable this boundary condition acts on
  const MooseLinearVariableFV<Real> & _moose_var;
  /// The Kokkos variable this boundary condition acts on
  Variable _var;

  /// Dispatcher for the right-hand side computation loop
  std::unique_ptr<DispatcherBase> _rhs_dispatcher;
  /// Dispatcher for the matrix computation loop
  std::unique_ptr<DispatcherBase> _matrix_dispatcher;

  /// Shallow copies of the owning flux kernel's BoundaryFaceData arrays, written by BC dispatches
  Array2D<Real> _bc_data_matrix;
  Array2D<Real> _bc_data_rhs;
};

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVBoundaryCondition::operator()(RightHandSideLoop,
                                      const ThreadID tid,
                                      const Derived & bc) const
{
  const auto [elem, side] = kokkosBoundaryElementSideID(tid);
  FVDatum datum(elem, side, kokkosMesh());
  _bc_data_rhs(side, elem) = bc.template computeRightHandSideContribution<Derived>(datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVBoundaryCondition::operator()(MatrixLoop, const ThreadID tid, const Derived & bc) const
{
  const auto [elem, side] = kokkosBoundaryElementSideID(tid);
  FVDatum datum(elem, side, kokkosMesh());
  _bc_data_matrix(side, elem) = bc.template computeMatrixContribution<Derived>(datum);
}

} // namespace Moose::Kokkos
