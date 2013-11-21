#include "NanKernel.h"

template<>
InputParameters validParams<NanKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<unsigned int>("timestep_to_nan", 1, "The timestep number to throw a nan on");
  return params;
}


NanKernel::NanKernel(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _timestep_to_nan(getParam<unsigned int>("timestep_to_nan"))
{
}

Real
NanKernel::computeQpResidual()
{
  Real zero = 0;

  if (static_cast<unsigned int>(_t_step) >= _timestep_to_nan)
    return _grad_u[_qp] * _grad_test[_i][_qp] / zero;
  else
    return _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
NanKernel::computeQpJacobian()
{
  return _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
