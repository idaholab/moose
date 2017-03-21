/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "NodalEqualValueConstraint.h"
#include "Assembly.h"

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
    mooseError(name(), ": The number of nodes has to be 2, but it is ", _node_ids.size(), ".");

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
