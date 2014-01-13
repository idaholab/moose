#include "DefaultPostprocessorDiffusion.h"

template<>
InputParameters validParams<DefaultPostprocessorDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<PostprocessorName>("pps_name", "the name of the postprocessor we are going to use");
  return params;
}


DefaultPostprocessorDiffusion::DefaultPostprocessorDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _pps_value(getPostprocessorValue("pps_name", 0.1))
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
