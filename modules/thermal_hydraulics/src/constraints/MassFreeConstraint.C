//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  return params;
}

MassFreeConstraint::MassFreeConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _normals(getParam<std::vector<Real>>("normals")),
    _rhouA(coupledValue("rhouA")),
    _rhouA_var_number(coupled("rhouA"))
{
  _primary_node_vector = getParam<std::vector<dof_id_type>>("nodes");
  // just a dummy value that is never used
  _connected_nodes.push_back(*_primary_node_vector.begin());
}

void
MassFreeConstraint::computeResidual(NumericVector<Number> & /*residual*/)
{
  auto && dofs = _var.dofIndices();
  DenseVector<Number> re(dofs.size());

  re.zero();
  for (unsigned int i = 0; i < dofs.size(); i++)
    re(i) = _rhouA[i] * _normals[i];
  re *= _var.scalingFactor();
  _assembly.cacheResidualNodes(re, dofs);
}

Real MassFreeConstraint::computeQpResidual(Moose::ConstraintType /*type*/) { return 0; }

void
MassFreeConstraint::computeJacobian(SparseMatrix<Number> & /*jacobian*/)
{
  auto && dofs = _var.dofIndices();

  {
    DenseMatrix<Number> Kee(dofs.size(), dofs.size());
    Kee.zero();
    _assembly.cacheJacobianBlock(Kee, dofs, dofs, _var.scalingFactor());
  }

  // off-diag
  {
    MooseVariable & var_rhouA = _sys.getFieldVariable<Real>(_tid, _rhouA_var_number);
    auto && dofs_rhouA = var_rhouA.dofIndices();
    DenseMatrix<Number> Kee(dofs.size(), dofs_rhouA.size());

    Kee.zero();
    for (unsigned int i = 0; i < dofs.size(); i++)
      Kee(i, i) = _normals[i];
    _assembly.cacheJacobianBlock(Kee, dofs, dofs_rhouA, _var.scalingFactor());
  }
}

Real MassFreeConstraint::computeQpJacobian(Moose::ConstraintJacobianType /*type*/) { return 0; }
