#include "TravelingWaveDoubleObstcle.h"

registerMooseObject("PhaseFieldApp", TravelingWaveDoubleObstcle);

InputParameters
TravelingWaveDoubleObstcle::validParams()
{
  InputParameters params = KernelValue::validParams();
  params.addRequiredParam<MaterialPropertyName>("sigma", "Interface energy");
  params.addRequiredParam<MaterialPropertyName>("eta", "Interface width");
  params.addRequiredParam<MaterialPropertyName>("delta_g", "Driving force");
  params.addClassDescription("Traveling wave kenel for double obstacle potential,"
                             "-gamma * (phi - 0.5) - sqrt(phi * (1 - phi)) * m.");
  return params;
}

TravelingWaveDoubleObstcle::TravelingWaveDoubleObstcle(const InputParameters & parameters)
  : KernelValue(parameters),
    _sigma(getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("sigma"))),
    _eta(getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("eta"))),
    _delta_g(getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("delta_g")))
{
}

Real
TravelingWaveDoubleObstcle::precomputeQpResidual()
{
  Real res = -8.0 * _sigma[_qp] / _eta[_qp] * (_u[_qp] - 0.5);
  if (_u[_qp] * (1.0 - _u[_qp]) > 1.0e-16)
    res -= -8.0 / libMesh::pi / _delta_g[_qp] * std::sqrt(_u[_qp] * (1.0 - _u[_qp]));
  return res;
}

Real
TravelingWaveDoubleObstcle::precomputeQpJacobian()
{
  Real jac = -8.0 * _sigma[_qp] / _eta[_qp] * _phi[_j][_qp];
  if (_u[_qp] * (1.0 - _u[_qp]) > 1.0e-16)
    jac -= -4.0 / libMesh::pi / _delta_g[_qp] * (1.0 - 2 * _u[_qp]) * _phi[_j][_qp] / std::sqrt(_u[_qp] * (1.0 - _u[_qp]));
  return jac;
}
