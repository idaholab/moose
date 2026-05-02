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

class LinearFVKernel : public LinearSystemContributionObject,
                       public BlockRestrictable,
                       public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  LinearFVKernel(const InputParameters & parameters);
  LinearFVKernel(const LinearFVKernel & object);

  struct RightHandSideLoop
  {
  };
  struct MatrixLoop
  {
  };

  virtual void computeRightHandSide() = 0;
  virtual void computeMatrix() = 0;

protected:
  Variable _kokkos_var;

  std::unique_ptr<DispatcherBase> _rhs_dispatcher;
  std::unique_ptr<DispatcherBase> _matrix_dispatcher;
};

class LinearFVElementalKernel : public LinearFVKernel
{
public:
  static InputParameters validParams();

  LinearFVElementalKernel(const InputParameters & parameters);
  LinearFVElementalKernel(const LinearFVElementalKernel & object);

  virtual void computeRightHandSide() override;
  virtual void computeMatrix() override;

  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(RightHandSideLoop, const ThreadID tid, const Derived & kernel) const;

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

  struct BoundaryFaceData
  {
    Array2D<Real> matrix_coeff;
    Array2D<Real> rhs_coeff;
  };

  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(RightHandSideLoop, const ThreadID tid, const Derived & kernel) const;

  template <typename Derived>
  KOKKOS_FUNCTION void operator()(MatrixLoop, const ThreadID tid, const Derived & kernel) const;

protected:
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
