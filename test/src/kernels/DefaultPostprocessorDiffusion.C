#include "DefaultPostprocessorDiffusion.h"

template<>
InputParameters validParams<DefaultPostprocessorDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addPostprocessorParam("pps_name", 0.1, "The name of the postprocessor we are going to use, if the name is not found a default value of 0.1 is utlized for the postprocessor value");
  return params;
}


DefaultPostprocessorDiffusion::DefaultPostprocessorDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _pps_value(getPostprocessorValue("pps_name"))
{
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
