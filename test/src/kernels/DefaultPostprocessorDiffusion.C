#include "DefaultPostprocessorDiffusion.h"

template<>
InputParameters validParams<DefaultPostprocessorDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addPostprocessor("pps_name", 0.1, "The name of the postprocessor we are going to use, if the name is not found a default value of 0.1 is utlized for the postprocessor value");
  params.addParam<bool>("test_default_error", false, "Set this to true to test the hasDefaultPostprocessorValue error message");
  return params;
}


DefaultPostprocessorDiffusion::DefaultPostprocessorDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _pps_value(getPostprocessorValue("pps_name"))
{
  // Test the error message for defaultPostprocessorValue
  if (getParam<bool>("test_default_error"))
    parameters.defaultPostprocessorValue("invalid_postprocessor_name");
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
