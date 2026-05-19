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
 * The base class for a user to derive their own Kokkos nodal boundary conditions using automatic
 * differentiation (AD).
 *
 * The user should define computeQpResidual() as inlined public methods in their derived class (not
 * virtual override). The signature of computeQpResidual() expected to be defined in the derived
 * class is as follows:
 *
 * @tparam Derived The object type
 * @param qp The dummy quadrature point index (= 0)
 * @param datum The AssemblyDatum object of the current thread
 * @returns The residual contribution
 *
 * template <typename Derived>
 * KOKKOS_FUNCTION Moose::Kokkos::ADReal computeQpResidual(const unsigned int qp,
 *                                                         AssemblyDatum & datum) const;
 *
 * Note that computeQpJacobian() and computeQpOffDiagJacobian() are unused for AD nodal boundary
 * conditions.
 */
class ADNodalBC : public NodalBCBase
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  ADNodalBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeResidualAndJacobian() override;

  /**
   * The parallel computation entry function called by Kokkos
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const;

protected:
  /**
   * Dispatch parallel calculation
   */
  virtual void dispatch();

  /**
   * Current solution at nodes
   */
  const ADVariableValue _u;
  /**
   * Whether computing residual
   */
  bool _computing_residual = false;
  /**
   * Whether computing Jacobian
   */
  bool _computing_jacobian = false;
};

template <typename Derived>
KOKKOS_FUNCTION void
ADNodalBC::operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const
{
  auto node = kokkosBoundaryNodeID(tid);
  auto & sys = kokkosSystem(_kokkos_var.sys());

  if (!sys.isNodalDefined(node, _kokkos_var.var()))
    return;

  AssemblyDatum datum(node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  datum.do_derivatives(_computing_jacobian);

  ADReal local_re = bc.template computeQpResidual<Derived>(0, datum);

  if (_computing_residual)
    accumulateTaggedNodalResidual(false, local_re.value(), node);
  if (_computing_jacobian)
    accumulateTaggedNodalMatrix(false, local_re.derivatives(), node);
}

} // namespace Moose::Kokkos
