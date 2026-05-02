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

  struct RightHandSideLoop
  {
  };
  struct MatrixLoop
  {
  };

  virtual void computeRightHandSide();
  virtual void computeMatrix();
  void initBCData(const LinearFVFluxKernel::BoundaryFaceData & data);

  template <typename Derived>
  KOKKOS_FUNCTION void operator()(RightHandSideLoop, const ThreadID tid, const Derived & bc) const;

  template <typename Derived>
  KOKKOS_FUNCTION void operator()(MatrixLoop, const ThreadID tid, const Derived & bc) const;

  const MooseLinearVariableFV<Real> & mooseVariable() const { return _moose_var; }

  Variable variable() const { return _var; }

protected:
  const MooseLinearVariableFV<Real> & _moose_var;
  Variable _var;

  std::unique_ptr<DispatcherBase> _rhs_dispatcher;
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
