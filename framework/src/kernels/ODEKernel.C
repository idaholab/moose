//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ODEKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<ODEKernel>()
{
  InputParameters params = validParams<ScalarKernel>();
  return params;
}

ODEKernel::ODEKernel(const InputParameters & parameters) : ScalarKernel(parameters) {}

void
ODEKernel::reinit()
{
}

void
ODEKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  for (_i = 0; _i < _var.order(); _i++)
    re(_i) += computeQpResidual();
}

void
ODEKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());

  for (_i = 0; _i < _var.order(); _i++)
    for (_j = 0; _j < _var.order(); _j++)
      ke(_i, _j) += computeQpJacobian();

  // compute off-diagonal jacobians wrt scalar variables
  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
  for (const auto & var : scalar_vars)
    computeOffDiagJacobian(var->number());
}

void
ODEKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (_sys.isScalarVariable(jvar))
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);
    MooseVariableScalar & var_j = _sys.getScalarVariable(_tid, jvar);
    for (_i = 0; _i < _var.order(); _i++)
      for (_j = 0; _j < var_j.order(); _j++)
      {
        if (jvar != _var.number())
          ke(_i, _j) += computeQpOffDiagJacobian(jvar);
      }
  }
}

Real
ODEKernel::computeQpJacobian()
{
  return 0.;
}

Real
ODEKernel::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
