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

template <>
InputParameters
validParams<NodalEqualValueConstraint>()
{
  InputParameters params = validParams<NodalScalarKernel>();
  params.addRequiredCoupledVar("var", "Variable(s) to put the constraint on");
  return params;
}

NodalEqualValueConstraint::NodalEqualValueConstraint(const InputParameters & parameters)
  : NodalScalarKernel(parameters)
{
  if (_node_ids.size() != 2)
    paramError("nodes", "invalid number of nodes: want 2, got ", _node_ids.size());

  unsigned int n = coupledComponents("var");
  _value.resize(n);
  _val_number.resize(n);
  for (unsigned int k = 0; k < n; k++)
  {
    _value[k] = &coupledValue("var", k);
    _val_number[k] = coupled("var", k);
  }
}

void
NodalEqualValueConstraint::computeResidual()
{
  // LM residuals
  DenseVector<Number> & lm_re = _assembly.residualBlock(_var.number());

  for (unsigned int k = 0; k < _value.size(); k++)
    lm_re(k) = (*_value[k])[0] - (*_value[k])[1];
}

void
NodalEqualValueConstraint::computeJacobian()
{
  // do the diagonal block
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  // put zeroes on the diagonal (we have to do it, otherwise PETSc will complain!)
  for (unsigned int i = 0; i < ke.m(); i++)
    for (unsigned int j = 0; j < ke.n(); j++)
      ke(i, j) = 0.;

  for (unsigned int k = 0; k < _value.size(); k++)
  {
    DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), _val_number[k]);

    ken(k, 0) = 1.;
    ken(k, 1) = -1.;
  }
}
