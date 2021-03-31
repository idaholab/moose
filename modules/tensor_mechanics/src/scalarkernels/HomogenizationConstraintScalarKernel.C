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
  params.addRequiredParam<unsigned int>("ndim",
                                        "Number of problem"
                                        " displacements");
  params.addRequiredParam<UserObjectName>("integrator",
                                          "The integrator user object doing the "
                                          "element calculations.");
  params.addParam<bool>("large_kinematics", false, "Are we using large deformations?");

  return params;
}

HomogenizationConstraintScalarKernel::HomogenizationConstraintScalarKernel(
    const InputParameters & parameters)
  : ScalarKernel(parameters),
    _ld(getParam<bool>("large_kinematics")),
    _ndisp(getParam<unsigned int>("ndim")),
    _ncomps(HomogenizationConstants::required.at(_ld)[_ndisp - 1]),
    _integrator(getUserObject<HomogenizationConstraintIntegral>("integrator")),
    _indices(HomogenizationConstants::indices.at(_ld)[_ndisp - 1]),
    _residual(_integrator.getResidual()),
    _jacobian(_integrator.getJacobian())
{
  if (_var.order() != _ncomps)
  {
    mooseError("Homogenization kernel requires a variable of order ", _ncomps);
  }
}

void
HomogenizationConstraintScalarKernel::reinit()
{
}

void
HomogenizationConstraintScalarKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  for (_i = 0; _i < re.size(); _i++)
    re(_i) += _residual(_indices[_i].first, _indices[_i].second);
}

void
HomogenizationConstraintScalarKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  for (_i = 0; _i < ke.m(); _i++)
    for (_j = 0; _j < ke.m(); _j++)
      ke(_i, _j) += _jacobian(
          _indices[_i].first, _indices[_i].second, _indices[_j].first, _indices[_j].second);
}

void
HomogenizationConstraintScalarKernel::computeOffDiagJacobian(unsigned int jvar)
{
}
