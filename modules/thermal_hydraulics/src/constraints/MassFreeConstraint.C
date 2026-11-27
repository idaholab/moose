//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFreeConstraint.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "MooseVariable.h"

registerMooseObject("ThermalHydraulicsApp", MassFreeConstraint);

InputParameters
MassFreeConstraint::validParams()
{
  InputParameters params = NodalConstraint::validParams();
  params.addRequiredParam<std::vector<Real>>("normals", "node normals");
  params.addRequiredParam<std::vector<dof_id_type>>("nodes", "node IDs");
  params.addRequiredCoupledVar("rhouA", "Momentum");
  params.addClassDescription(
      "Constrains the momentum at the user-specified nodes along the user-specified normals");
  return params;
}

MassFreeConstraint::MassFreeConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _normals(getParam<std::vector<Real>>("normals")),
    _rhouA(coupledValue("rhouA")),
    _rhouA_var_number(coupled("rhouA"))
{
  if (_normals.size() != getParam<std::vector<dof_id_type>>("nodes").size())
    paramError("normals", "'normals' must have as many entries as 'nodes'.");

  updateConstrainedNodes();
}

void
MassFreeConstraint::updateConnectivity()
{
  // _normals is indexed by position in this list, so every process keeps all of the nodes in the
  // order the user gave them. updateConstrainedNodes() ghosts the elements that keep them
  // available on a distributed mesh.
  _primary_node_vector = getParam<std::vector<dof_id_type>>("nodes");

  // This constraint has no secondary node: it overrides computeResidual() and computeJacobian()
  // and only ever assembles on _primary_node_vector. The value here is never read, but
  // _connected_nodes has to be non-empty, because NonlinearSystemBase skips a nodal constraint
  // whose secondary node list is empty.
  _connected_nodes.push_back(*_primary_node_vector.begin());
}

void
MassFreeConstraint::computeResidual(const NumericVector<Number> & /*residual*/)
{
  const auto & dofs = _var.dofIndices();
  std::vector<Number> re(dofs.size());

  for (unsigned int i = 0; i < dofs.size(); i++)
    re[i] = _rhouA[i] * _normals[i];

  addResiduals(_assembly, re, dofs, _var.scalingFactor());
}

Real
MassFreeConstraint::computeQpResidual(Moose::ConstraintType /*type*/)
{
  return 0;
}

void
MassFreeConstraint::computeJacobian(const SparseMatrix<Number> & /*jacobian*/)
{
  const auto & dofs = _var.dofIndices();

  // off-diag
  {
    MooseVariable & var_rhouA = _sys.getFieldVariable<Real>(_tid, _rhouA_var_number);
    auto && dofs_rhouA = var_rhouA.dofIndices();
    DenseMatrix<Number> Kee(dofs.size(), dofs_rhouA.size());

    Kee.zero();
    for (unsigned int i = 0; i < dofs.size(); i++)
      Kee(i, i) = _normals[i];
    addJacobian(_assembly, Kee, dofs, dofs_rhouA, _var.scalingFactor());
  }
}

Real
MassFreeConstraint::computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
{
  return 0;
}
