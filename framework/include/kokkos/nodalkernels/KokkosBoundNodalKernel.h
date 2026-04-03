//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalKernel.h"

class KokkosBoundNodalKernel : public Moose::Kokkos::NodalKernel
{
public:
  static InputParameters validParams();

  KokkosBoundNodalKernel(const InputParameters & parameters);

  virtual void initialSetup() override;

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int jvar,
                                                const unsigned int qp,
                                                AssemblyDatum & datum) const;

protected:
  /// The number of the coupled variable
  const unsigned int _v_var;

  /// The value of the coupled variable
  const Moose::Kokkos::VariableValue _v;

private:
  KOKKOS_FUNCTION bool skipOnBoundary(const ContiguousNodeID node) const;

  /// Boundaries on which we should not execute this object
  Moose::Kokkos::Array<ContiguousBoundaryID> _bnd_ids;
};

KOKKOS_FUNCTION inline bool
KokkosBoundNodalKernel::skipOnBoundary(const ContiguousNodeID node) const
{
  for (dof_id_type b = 0; b < _bnd_ids.size(); ++b)
    if (kokkosMesh().isBoundaryNode(node, _bnd_ids[b]))
      return true;

  return false;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBoundNodalKernel::computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  if (skipOnBoundary(datum.node()))
    return _u(datum, qp);

  return static_cast<const Derived *>(this)->getResidual(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBoundNodalKernel::computeQpJacobian(const unsigned int qp, AssemblyDatum & datum) const
{
  if (skipOnBoundary(datum.node()))
    return 1;

  return static_cast<const Derived *>(this)->getJacobian(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBoundNodalKernel::computeQpOffDiagJacobian(const unsigned int jvar,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const
{
  if (skipOnBoundary(datum.node()))
    return 0;

  return static_cast<const Derived *>(this)->getOffDiagJacobian(jvar, qp, datum);
}
