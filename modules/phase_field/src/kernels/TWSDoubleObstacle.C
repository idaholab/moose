#include "TWSDoubleObstacle.h"

registerMooseObject("PhaseFieldApp", TWSDoubleObstacle);

InputParameters
TWSDoubleObstacle::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addRequiredParam<std::string>("sigma", "the interface energy used in the kernel");
  params.addRequiredParam<std::string>("eta", "the interface width used in the kernel");
  params.addRequiredParam<std::string>("delta_g", "the driving force used in the kernel");
  params.addClassDescription("traveling wave kenel for double obstacle potential,"
                             "-gamma * (phi - 0.5) - sqrt(phi * (1 - phi)) * m.");
  return params;
}

TWSDoubleObstacle::TWSDoubleObstacle(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _sigma_name(getParam<std::string>("sigma")),
    _sigma(getADMaterialProperty<Real>(_sigma_name)),
    _eta_name(getParam<std::string>("eta")),
    _eta(getADMaterialProperty<Real>(_eta_name)),
    _delta_g_name(getParam<std::string>("delta_g")),
    _delta_g(getADMaterialProperty<Real>(_delta_g_name))
{
}

ADReal
TWSDoubleObstacle::precomputeQpResidual()
{
  auto value = -8.0 * _sigma[_qp] / _eta[_qp] * (_u[_qp] - 0.5);
  if (_u[_qp] * (1.0 - _u[_qp]) > 1.0e-16)
    value -= -8.0 / _pi / _delta_g[_qp] * std::sqrt(_u[_qp] * (1.0 - _u[_qp]));
  return value;
}