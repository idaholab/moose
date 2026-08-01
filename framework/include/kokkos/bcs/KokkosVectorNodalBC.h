//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalBCBase.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos nodal boundary conditions on vector
 * variables.
 */
class VectorNodalBC : public NodalBCBase
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  VectorNodalBC(const InputParameters & parameters);

  /**
   * Dispatch residual calculation
   */
  virtual void computeResidual() override;
  /**
   * Dispatch diagonal and off-diagonal Jacobian calculation
   */
  virtual void computeJacobian() override;

  /**
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION Real3 computeQpJacobian(const unsigned int /* qp */,
                                          AssemblyDatum & /* datum */) const
  {
    return Real3(1);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* jvar */,
                                                const unsigned int /* qp */,
                                                AssemblyDatum & /* datum */) const
  {
    ::Kokkos::abort(
        "Default computeQpOffDiagJacobian() should never be called. Make sure you properly "
        "redefined this method in your class without typos.");

    return 0;
  }
  ///@}

  /**
   * Functions used to check if users have overriden the hook methods, whose calculations can be
   * skipped when not overriden
   */
  ///@{
  template <typename Derived>
  static auto defaultJacobian()
  {
    return &VectorNodalBC::computeQpJacobian<Derived>;
  }
  template <typename Derived>
  static auto defaultOffDiagJacobian()
  {
    return &VectorNodalBC::computeQpOffDiagJacobian<Derived>;
  }
  ///@}

  /**
   * The parallel computation entry functions called by Kokkos
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(JacobianLoop, const ThreadID tid, const Derived & bc) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & bc) const;
  ///@}

protected:
  /**
   * Current vector solution at nodes
   */
  const VectorVariableValue _u;
};

template <typename Derived>
KOKKOS_FUNCTION void
VectorNodalBC::operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const
{
  auto node = kokkosBoundaryNodeID(tid);
  auto & sys = kokkosSystem(_kokkos_var.sys());

  if (!sys.isNodalDefined(node, _kokkos_var.var()))
    return;

  AssemblyDatum datum(node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  Real3 local_re = bc.template computeQpResidual<Derived>(0, datum);

  for (unsigned int comp = 0; comp < _dimension; ++comp)
    accumulateTaggedVectorNodalResidual(false, local_re(comp), node, comp);
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorNodalBC::operator()(JacobianLoop, const ThreadID tid, const Derived & bc) const
{
  auto node = kokkosBoundaryNodeID(tid);
  auto & sys = kokkosSystem(_kokkos_var.sys());

  if (!sys.isNodalDefined(node, _kokkos_var.var()))
    return;

  AssemblyDatum datum(node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  Real3 local_ke = bc.template computeQpJacobian<Derived>(0, datum);

  for (unsigned int comp = 0; comp < _dimension; ++comp)
    accumulateTaggedVectorNodalMatrix(false, local_ke(comp), node, comp, comp, _kokkos_var.var());
}

template <typename Derived>
KOKKOS_FUNCTION void
VectorNodalBC::operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & bc) const
{
  auto node = kokkosBoundaryNodeID(_thread(tid, 1));
  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getCoupling(_kokkos_var.var())[_thread(tid, 0)];

  if (!sys.isNodalDefined(node, _kokkos_var.var()))
    return;

  AssemblyDatum datum(node, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar);

  Real3 local_ke = bc.template computeQpOffDiagJacobian<Derived>(jvar, 0, datum);

  for (unsigned int comp = 0; comp < _dimension; ++comp)
    accumulateTaggedVectorNodalMatrix(true, local_ke(comp), node, comp, 0, jvar);
}

} // namespace Moose::Kokkos
