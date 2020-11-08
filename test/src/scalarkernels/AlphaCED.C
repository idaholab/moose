//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AlphaCED.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"

registerMooseObject("MooseTestApp", AlphaCED);

InputParameters
AlphaCED::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addRequiredParam<Real>("value", "The value we are enforcing");

  return params;
}

AlphaCED::AlphaCED(const InputParameters & parameters)
  : ScalarKernel(parameters), _value(getParam<Real>("value"))
{
}

void
AlphaCED::reinit()
{
}

void
AlphaCED::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  for (_i = 0; _i < re.size(); _i++)
    re(_i) += computeQpResidual();
}

Real
AlphaCED::computeQpResidual()
{
  return _u[_i] - _value;
}

void
AlphaCED::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  for (_i = 0; _i < ke.m(); _i++)
    ke(_i, _i) += computeQpJacobian();
}

Real
AlphaCED::computeQpJacobian()
{
  return 1.;
}

void
AlphaCED::computeOffDiagJacobianScalar(unsigned int /*jvar*/)
{
}

Real
AlphaCED::computeQpOffDiagJacobianScalar(unsigned int /*jvar*/)
{
  return 0.;
}
