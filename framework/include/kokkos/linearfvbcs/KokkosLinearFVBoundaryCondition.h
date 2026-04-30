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
#include "MooseLinearVariableFV.h"
#include "MooseVariableDependencyInterface.h"
#include "NonADFunctorInterface.h"
#include "FaceArgInterface.h"

#include "KokkosLinearFVKernel.h"

namespace Moose::Kokkos
{

class LinearFVBoundaryCondition : public LinearSystemContributionObject,
                                  public BoundaryRestrictableRequired,
                                  public GeometricSearchInterface,
                                  public MooseVariableDependencyInterface,
                                  public NonADFunctorInterface,
                                  public FaceArgProducerInterface
{
public:
  static InputParameters validParams();

  LinearFVBoundaryCondition(const InputParameters & parameters);
  LinearFVBoundaryCondition(const LinearFVBoundaryCondition & object);

  virtual const MooseLinearVariableFV<Real> & variable() const { return _var; }
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

  KOKKOS_FUNCTION Real computeRightHandSideContribution(const AssemblyDatum &) const { return 0; }
  KOKKOS_FUNCTION Real computeMatrixContribution(const AssemblyDatum &) const { return 0; }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeRightHandSideContributionShim(const Derived & bc,
                                                            const AssemblyDatum & datum) const
  {
    return bc.computeRightHandSideContribution(datum);
  }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeMatrixContributionShim(const Derived & bc,
                                                     const AssemblyDatum & datum) const
  {
    return bc.computeMatrixContribution(datum);
  }

  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(RightHandSideLoop, const ThreadID tid, const Derived & bc) const
  {
    const auto [elem, side] = kokkosBoundaryElementSideID(tid);
    AssemblyDatum datum(
        elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());
    _bc_data_rhs(side, elem) = bc.computeRightHandSideContributionShim(bc, datum);
  }

  template <typename Derived>
  KOKKOS_FUNCTION void operator()(MatrixLoop, const ThreadID tid, const Derived & bc) const
  {
    const auto [elem, side] = kokkosBoundaryElementSideID(tid);
    AssemblyDatum datum(
        elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());
    _bc_data_matrix(side, elem) = bc.computeMatrixContributionShim(bc, datum);
  }

protected:
  MooseLinearVariableFV<Real> & _var;
  Variable _kokkos_var;

  const unsigned int _var_num;
  const unsigned int _sys_num;

  std::unique_ptr<DispatcherBase> _rhs_dispatcher;
  std::unique_ptr<DispatcherBase> _matrix_dispatcher;

  /// Shallow copies of the owning flux kernel's BoundaryFaceData arrays, written by BC dispatches
  Array2D<Real> _bc_data_matrix;
  Array2D<Real> _bc_data_rhs;
};

} // namespace Moose::Kokkos
