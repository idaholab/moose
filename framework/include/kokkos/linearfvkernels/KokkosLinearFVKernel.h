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
#include "FaceArgInterface.h"
#include "FVRelationshipManagerInterface.h"
#include "MooseLinearVariableFV.h"
#include "MooseVariableDependencyInterface.h"
#include "NonADFunctorInterface.h"

namespace Moose::Kokkos
{

class LinearFVKernel : public LinearSystemContributionObject,
                       public BlockRestrictable,
                       public NonADFunctorInterface,
                       public MooseVariableDependencyInterface
{
public:
  static InputParameters validParams();

  LinearFVKernel(const InputParameters & parameters);
  LinearFVKernel(const LinearFVKernel & object);

  virtual const MooseLinearVariableFV<Real> & variable() const { return _var; }

  struct RightHandSideLoop
  {
  };
  struct MatrixLoop
  {
  };

  virtual void computeRightHandSide() = 0;
  virtual void computeMatrix() = 0;

protected:
  MooseLinearVariableFV<Real> & _var;
  Variable _kokkos_var;

  std::unique_ptr<DispatcherBase> _rhs_dispatcher;
  std::unique_ptr<DispatcherBase> _matrix_dispatcher;

  const unsigned int _var_num;
  const unsigned int _sys_num;
};

class LinearFVElementalKernel : public LinearFVKernel
{
public:
  static InputParameters validParams();

  LinearFVElementalKernel(const InputParameters & parameters);
  LinearFVElementalKernel(const LinearFVElementalKernel & object);

  virtual void computeRightHandSide() override;
  virtual void computeMatrix() override;

  KOKKOS_FUNCTION Real computeRightHandSideContribution(const AssemblyDatum &) const { return 0; }
  KOKKOS_FUNCTION Real computeMatrixContribution(const AssemblyDatum &) const { return 0; }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeRightHandSideContributionShim(const Derived & kernel,
                                                            const AssemblyDatum & datum) const
  {
    return kernel.computeRightHandSideContribution(datum);
  }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeMatrixContributionShim(const Derived & kernel,
                                                     const AssemblyDatum & datum) const
  {
    return kernel.computeMatrixContribution(datum);
  }

  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(RightHandSideLoop, const ThreadID tid, const Derived & kernel) const
  {
    const auto elem = kokkosBlockElementID(tid);
    AssemblyDatum datum(elem,
                        libMesh::invalid_uint,
                        kokkosAssembly(),
                        kokkosSystems(),
                        _kokkos_var,
                        _kokkos_var.var());
    const auto & sys = kokkosSystem(_sys.number());
    kernel.accumulateTaggedVector(kernel.computeRightHandSideContributionShim(kernel, datum),
                                  sys.getElemLocalDofIndex(elem, 0, _kokkos_var.var()));
  }

  template <typename Derived>
  KOKKOS_FUNCTION void operator()(MatrixLoop, const ThreadID tid, const Derived & kernel) const
  {
    const auto elem = kokkosBlockElementID(tid);
    AssemblyDatum datum(elem,
                        libMesh::invalid_uint,
                        kokkosAssembly(),
                        kokkosSystems(),
                        _kokkos_var,
                        _kokkos_var.var());
    const auto & sys = kokkosSystem(_sys.number());
    const auto row = sys.getElemLocalDofIndex(elem, 0, _kokkos_var.var());
    kernel.accumulateTaggedMatrix(kernel.computeMatrixContributionShim(kernel, datum),
                                  row,
                                  sys.getElemGlobalDofIndex(elem, 0, _kokkos_var.var()));
  }
};

class LinearFVFluxKernel : public LinearFVKernel, public FaceArgProducerInterface
{
public:
  static InputParameters validParams();

  LinearFVFluxKernel(const InputParameters & parameters);
  LinearFVFluxKernel(const LinearFVFluxKernel & object);

  virtual void computeRightHandSide() override;
  virtual void computeMatrix() override;
  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override;

  KOKKOS_FUNCTION Real computeRightHandSideContribution(const AssemblyDatum &) const { return 0; }
  KOKKOS_FUNCTION Real computeMatrixContribution(const AssemblyDatum &) const { return 0; }
  KOKKOS_FUNCTION Real computeNeighborMatrixContribution(const AssemblyDatum &) const { return 0; }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeRightHandSideContributionShim(const Derived & kernel,
                                                            const AssemblyDatum & datum) const
  {
    return kernel.computeRightHandSideContribution(datum);
  }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeMatrixContributionShim(const Derived & kernel,
                                                     const AssemblyDatum & datum) const
  {
    return kernel.computeMatrixContribution(datum);
  }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeNeighborMatrixContributionShim(const Derived & kernel,
                                                             const AssemblyDatum & datum) const
  {
    return kernel.computeNeighborMatrixContribution(datum);
  }

  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(RightHandSideLoop, const ThreadID tid, const Derived & kernel) const
  {
    const auto [elem, side] = kokkosBlockElementSideID(tid);
    AssemblyDatum datum(
        elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());
    const auto & sys = kokkosSystem(_sys.number());
    kernel.accumulateTaggedVector(kernel.computeRightHandSideContributionShim(kernel, datum),
                                  sys.getElemLocalDofIndex(elem, 0, _kokkos_var.var()));
  }

  template <typename Derived>
  KOKKOS_FUNCTION void operator()(MatrixLoop, const ThreadID tid, const Derived & kernel) const
  {
    const auto [elem, side] = kokkosBlockElementSideID(tid);
    AssemblyDatum datum(
        elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());
    const auto & sys = kokkosSystem(_sys.number());
    const auto row = sys.getElemLocalDofIndex(elem, 0, _kokkos_var.var());
    kernel.accumulateTaggedMatrix(kernel.computeMatrixContributionShim(kernel, datum),
                                  row,
                                  sys.getElemGlobalDofIndex(elem, 0, _kokkos_var.var()));

    if (datum.hasNeighbor())
      kernel.accumulateTaggedMatrix(
          kernel.computeNeighborMatrixContributionShim(kernel, datum),
          row,
          sys.getElemGlobalDofIndex(datum.mesh().getNeighbor(elem, side), 0, _kokkos_var.var()));
  }

protected:
  const bool _force_boundary_execution;
};

} // namespace Moose::Kokkos
