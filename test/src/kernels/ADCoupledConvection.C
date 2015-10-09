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
#include "ADCoupledConvection.h"

template<>
InputParameters validParams<ADCoupledConvection>()
{
  InputParameters params = validParams<ADKernel>();
  params.addRequiredCoupledVar("velocity_vector", "Velocity Vector for the Convection ADKernel");
  return params;
}

ADCoupledConvection::ADCoupledConvection(const InputParameters & parameters) :
  ADKernel(parameters),
  _velocity_vector(adCoupledGradient("velocity_vector"))
{}

ADReal
ADCoupledConvection::computeQpResidual()
{
  return _test[_i][_qp]*(_velocity_vector[_qp]*_grad_u[_qp]);
}
