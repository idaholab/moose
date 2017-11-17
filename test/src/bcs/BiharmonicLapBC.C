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

#include "BiharmonicLapBC.h"
#include "Function.h"

template <>
InputParameters
validParams<BiharmonicLapBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<FunctionName>(
      "laplacian_function", "0", "A function representing the weakly-imposed Laplacian.");
  return params;
}

BiharmonicLapBC::BiharmonicLapBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _lap_u(getFunction("laplacian_function"))
{
}

Real
BiharmonicLapBC::computeQpResidual()
{
  return -_lap_u.value(_t, _q_point[_qp]) * (_grad_test[_i][_qp] * _normals[_qp]);
}
