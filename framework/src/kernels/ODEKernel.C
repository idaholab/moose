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

InputParameters
ODEKernel::validParams()
{
  InputParameters params = ScalarKernel::validParams();
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
  prepareVectorTag(_assembly, _var.number());

  for (_i = 0; _i < _var.order(); _i++)
    _local_re(_i) += computeQpResidual();

  accumulateTaggedLocalResidual();
}

void
ODEKernel::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  for (_i = 0; _i < _var.order(); _i++)
    for (_j = 0; _j < _var.order(); _j++)
      _local_ke(_i, _j) += computeQpJacobian();

  accumulateTaggedLocalMatrix();

  // compute off-diagonal jacobians wrt scalar variables
  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
  for (const auto & var : scalar_vars)
    computeOffDiagJacobianScalar(var->number());
}

void
ODEKernel::computeOffDiagJacobianScalar(unsigned int jvar)
{
  prepareMatrixTag(_assembly, _var.number(), jvar);

  MooseVariableScalar & var_j = _sys.getScalarVariable(_tid, jvar);
  for (_i = 0; _i < _var.order(); _i++)
    for (_j = 0; _j < var_j.order(); _j++)
    {
      if (jvar != _var.number())
        _local_ke(_i, _j) += computeQpOffDiagJacobianScalar(jvar);
    }

  accumulateTaggedLocalMatrix();
}

Real
ODEKernel::computeQpJacobian()
{
  return 0.;
}
