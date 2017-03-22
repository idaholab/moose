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
#include "DefaultPostprocessorDiffusion.h"

template <>
InputParameters
validParams<DefaultPostprocessorDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<PostprocessorName>(
      "pps_name",
      0.1,
      "The name of the postprocessor we are going to use, if the name is not "
      "found a default value of 0.1 is utlized for the postprocessor value");
  params.addParam<bool>("test_default_error",
                        false,
                        "Set this to true to test the hasDefaultPostprocessorValue error message");
  return params;
}

DefaultPostprocessorDiffusion::DefaultPostprocessorDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _pps_value(getPostprocessorValue("pps_name"))
{
  // Test the error message for defaultPostprocessorValue
  if (getParam<bool>("test_default_error"))
    parameters.getDefaultPostprocessorValue("invalid_postprocessor_name");
}

Real
DefaultPostprocessorDiffusion::computeQpResidual()
{
  return _pps_value * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
DefaultPostprocessorDiffusion::computeQpJacobian()
{
  return _pps_value * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
