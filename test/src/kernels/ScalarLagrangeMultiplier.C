//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarLagrangeMultiplier.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ScalarLagrangeMultiplier>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier variable");

  return params;
}

ScalarLagrangeMultiplier::ScalarLagrangeMultiplier(const InputParameters & parameters)
  : Kernel(parameters), _lambda_var(coupledScalar("lambda")), _lambda(coupledScalarValue("lambda"))
{
}

ScalarLagrangeMultiplier::~ScalarLagrangeMultiplier() {}

Real
ScalarLagrangeMultiplier::computeQpResidual()
{
  return _lambda[0] * _test[_i][_qp];
}

void
ScalarLagrangeMultiplier::computeOffDiagJacobianScalar(unsigned int jvar)
{
  DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar);
  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar, _var.number());
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        Real value = _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
        ken(_i, _j) += value;
        kne(_j, _i) += value;
      }
}

Real
ScalarLagrangeMultiplier::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _lambda_var)
    return _test[_i][_qp];
  else
    return 0.;
}
