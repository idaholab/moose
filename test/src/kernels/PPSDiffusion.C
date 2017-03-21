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
#include "PPSDiffusion.h"

template <>
InputParameters
validParams<PPSDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<PostprocessorName>("pps_name",
                                             "the name of the postprocessor we are going to use");
  return params;
}

PPSDiffusion::PPSDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _pps_value(getPostprocessorValue("pps_name"))
{
}

Real
PPSDiffusion::computeQpResidual()
{
  return _pps_value * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
PPSDiffusion::computeQpJacobian()
{
  return _pps_value * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
