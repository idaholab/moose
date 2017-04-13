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
#include "FunctionPenaltyDirichletBC.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionPenaltyDirichletBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<Real>("penalty", "Penalty scalar");
  params.addRequiredParam<FunctionName>("function", "Forcing function");

  return params;
}

FunctionPenaltyDirichletBC::FunctionPenaltyDirichletBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _func(getFunction("function")), _p(getParam<Real>("penalty"))
{
}

Real
FunctionPenaltyDirichletBC::computeQpResidual()
{
  return _p * _test[_i][_qp] * (-_func.value(_t, _q_point[_qp]) + _u[_qp]);
}

Real
FunctionPenaltyDirichletBC::computeQpJacobian()
{
  return _p * _phi[_j][_qp] * _test[_i][_qp];
}
