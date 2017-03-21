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
#include "CoupledConvection.h"

template <>
InputParameters
validParams<CoupledConvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("velocity_vector", "Velocity Vector for the Convection Kernel");
  params.addParam<bool>(
      "lag_coupling",
      false,
      "Tells the object to use the old velocity vector instead of the current vector");
  return params;
}

CoupledConvection::CoupledConvection(const InputParameters & parameters)
  : Kernel(parameters),
    _velocity_vector(getParam<bool>("lag_coupling") ? coupledGradientOld("velocity_vector")
                                                    : coupledGradient("velocity_vector"))
{
}

Real
CoupledConvection::computeQpResidual()
{
  return _test[_i][_qp] * (_velocity_vector[_qp] * _grad_u[_qp]);
}

Real
CoupledConvection::computeQpJacobian()
{
  return _test[_i][_qp] * (_velocity_vector[_qp] * _grad_phi[_j][_qp]);
}
