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
#include "ConservativeAdvection.h"

template <>
InputParameters
validParams<ConservativeAdvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity vector");
  return params;
}

ConservativeAdvection::ConservativeAdvection(const InputParameters & parameters)
  : Kernel(parameters), _velocity(getParam<RealVectorValue>("velocity"))
{
}

Real
ConservativeAdvection::computeQpResidual()
{
  return -_u[_qp] * (_velocity * _grad_test[_i][_qp]);
}

Real
ConservativeAdvection::computeQpJacobian()
{
  return -_phi[_j][_qp] * (_velocity * _grad_test[_i][_qp]);
}
