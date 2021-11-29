//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalEqualValueConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"

registerMooseObject("MooseApp", NodalEqualValueConstraint);

InputParameters
NodalEqualValueConstraint::validParams()
{
  InputParameters params = NodalScalarKernel::validParams();
  params.addClassDescription("Constrain two nodes to have identical values.");
  params.addRequiredCoupledVar("var", "Variable(s) to put the constraint on");
  return params;
}

NodalEqualValueConstraint::NodalEqualValueConstraint(const InputParameters & parameters)
  : NodalScalarKernel(parameters), _val_number(coupledIndices("var")), _value(coupledValues("var"))
{
  if (_node_ids.size() != 2)
    paramError("boundary", "invalid number of nodes: want 2, got ", _node_ids.size());
}

void
NodalEqualValueConstraint::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  for (unsigned int k = 0; k < _value.size(); k++)
    _local_re(k) = (*_value[k])[0] - (*_value[k])[1];

  assignTaggedLocalResidual();
}

void
NodalEqualValueConstraint::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  // put zeroes on the diagonal (we have to do it, otherwise PETSc will complain!)
  for (unsigned int i = 0; i < _local_ke.m(); i++)
    for (unsigned int j = 0; j < _local_ke.n(); j++)
      _local_ke(i, j) = 0.;

  assignTaggedLocalMatrix();

  for (unsigned int k = 0; k < _value.size(); k++)
  {
    prepareMatrixTag(_assembly, _var.number(), _val_number[k]);

    _local_ke(k, 0) = 1.;
    _local_ke(k, 1) = -1.;

    assignTaggedLocalMatrix();
  }
}
