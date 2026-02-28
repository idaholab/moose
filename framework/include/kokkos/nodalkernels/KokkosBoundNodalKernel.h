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

template <typename Derived>
class KokkosBoundNodalKernel : public Moose::Kokkos::NodalKernel
{
public:
  static InputParameters validParams();

  KokkosBoundNodalKernel(const InputParameters & parameters);

  virtual void initialSetup() override;

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int qp, AssemblyDatum & datum) const;
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

template <typename Derived>
InputParameters
KokkosBoundNodalKernel<Derived>::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addRequiredCoupledVar(
      "v", "The coupled variable we require to be greater than the lower bound");
  params.addParam<std::vector<BoundaryName>>(
      "exclude_boundaries",
      {},
      "Boundaries on which not to execute the nodal kernel. This can be useful for avoiding "
      "singuarility in the matrix in case a constraint is active in the same place that a "
      "Dirichlet BC is set");
  return params;
}

template <typename Derived>
KokkosBoundNodalKernel<Derived>::KokkosBoundNodalKernel(const InputParameters & parameters)
  : NodalKernel(parameters), _v_var(coupled("v")), _v(kokkosCoupledNodalValue("v"))
{
  for (const auto var : variables())
    if (var->number() == _v_var)
      paramError("v", "Coupled variable needs to be different from variable");
}

template <typename Derived>
void
KokkosBoundNodalKernel<Derived>::initialSetup()
{
  std::set<ContiguousBoundaryID> bnd_ids;

  const auto & bnd_names = getParam<std::vector<BoundaryName>>("exclude_boundaries");
  for (const auto & bnd_id : _mesh.getBoundaryIDs(bnd_names))
    bnd_ids.insert(kokkosMesh().getContiguousBoundaryID(bnd_id));

  _bnd_ids = bnd_ids;
}

template <typename Derived>
KOKKOS_FUNCTION bool
KokkosBoundNodalKernel<Derived>::skipOnBoundary(const ContiguousNodeID node) const
{
  for (dof_id_type b = 0; b < _bnd_ids.size(); ++b)
    if (kokkosMesh().isBoundaryNode(node, _bnd_ids[b]))
      return true;

  return false;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBoundNodalKernel<Derived>::computeQpResidual(const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  if (skipOnBoundary(datum.node()))
    return _u(datum, qp);

  return static_cast<const Derived *>(this)->getResidual(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBoundNodalKernel<Derived>::computeQpJacobian(const unsigned int qp,
                                                   AssemblyDatum & datum) const
{
  if (skipOnBoundary(datum.node()))
    return 1;

  return static_cast<const Derived *>(this)->getJacobian(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBoundNodalKernel<Derived>::computeQpOffDiagJacobian(const unsigned int jvar,
                                                          const unsigned int qp,
                                                          AssemblyDatum & datum) const
{
  if (skipOnBoundary(datum.node()))
    return 0;

  return static_cast<const Derived *>(this)->getOffDiagJacobian(jvar, qp, datum);
}
