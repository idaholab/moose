//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HomogenizationConstraintScalarKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", HomogenizationConstraintScalarKernel);

InputParameters
HomogenizationConstraintScalarKernel::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("homogenization_constraint",
                                          "The UserObject defining the homogenization constraint");

  return params;
}

HomogenizationConstraintScalarKernel::HomogenizationConstraintScalarKernel(
    const InputParameters & parameters)
  : ScalarKernel(parameters),
    _constraint(getUserObject<HomogenizationConstraint>("homogenization_constraint")),
    _residual(_constraint.getResidual()),
    _jacobian(_constraint.getJacobian()),
    _cmap(_constraint.getConstraintMap())
{
  if (_var.order() != _cmap.size())
    mooseError("Homogenization kernel requires a variable of order ", _cmap.size());
}

void
HomogenizationConstraintScalarKernel::reinit()
{
}

void
HomogenizationConstraintScalarKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _i = 0;
  for (auto && indices : _cmap)
  {
    auto && [i, j] = indices.first;
    re(_i++) += _residual(i, j);
  }
}

void
HomogenizationConstraintScalarKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _i = 0;
  for (auto && indices1 : _cmap)
  {
    auto && [i, j] = indices1.first;
    _j = 0;
    for (auto && indices2 : _cmap)
    {
      auto && [a, b] = indices2.first;
      ke(_i, _j++) += _jacobian(i, j, a, b);
    }
    _i++;
  }
}
