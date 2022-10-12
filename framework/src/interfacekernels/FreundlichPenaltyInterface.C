#include "FreundlichPenaltyInterface.h"

registerMooseObject("MooseApp", FreundlichPenaltyInterface);

InputParameters
FreundlichPenaltyInterface::validParams()
{
  InputParameters params = ADInterfaceKernel::validParams();
  params.addParam<MaterialPropertyName>(
      "As",
      "As",
      " Temperature independent coefficient of concentration proportionality, intercept term");
  params.addParam<MaterialPropertyName>("Bs",
                                        "Bs",
                                        " Temperature-dependent coefficient of "
                                        "concentration-pressure proportionality, intercept term");
  params.addParam<MaterialPropertyName>(
      "Ds",
      "Ds",
      " Temperature independent coefficient of concentration-pressure proportionality order");
  params.addParam<MaterialPropertyName>(
      "Es",
      "Es",
      " Temperature dependence of order of proportion between temperature and concentration");
  params.addParam<MaterialPropertyName>(
      "d1s", "d1s", " Temperature independent coefficient of transition concentration.");
  params.addParam<MaterialPropertyName>(
      "d2s", "d2s", " Temperature dependence of transition concentration.");
  params.addParam<MaterialPropertyName>("rho", "rho", " Density of solid phase [g/cm^3]");
  params.addParam<MaterialPropertyName>("diffusivity", "D", " Diffusivity of solid phase [m/s^2]");
  params.addParam<MaterialPropertyName>("d_vapor", "D", " Diffusivity of vapor phase [m/s^2]");
  params.addParam<MaterialPropertyName>("rho", "rho", " Density of solid phase [g/cm^3]");
  params.addParam<Real>("penalty", 1, " penalty associated with concentration imbalance");
  params.addRequiredCoupledVar("T", "Temperature");

  return params;
}

FreundlichPenaltyInterface::FreundlichPenaltyInterface(const InputParameters & parameters)
  : ADInterfaceKernel(parameters),
    _A(getADMaterialProperty<Real>("As")),
    _B(getADMaterialProperty<Real>("Bs")),
    _D(getADMaterialProperty<Real>("Ds")),
    _E(getADMaterialProperty<Real>("Es")),
    _d1(getADMaterialProperty<Real>("d1s")),
    _d2(getADMaterialProperty<Real>("d2s")),
    _diff(getADMaterialProperty<Real>("diffusivity")),
    _diff_neighbor(getADMaterialProperty<Real>("d_vapor")),
    _carbon_density(getADMaterialProperty<Real>("rho")),
    _penalty(getParam<Real>("penalty")),
    _T_var(coupled("T")),
    _temperature(coupledValue("T"))
{
}
ADReal
FreundlichPenaltyInterface::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0;
  ADReal ln_trans_conc = ln_trans_conc = _d1[_qp] - _d2[_qp] * _temperature[_qp];
  ADReal subresidual = 0;
  switch (type)
  {
    case Moose::Element:
      r = log(_u[_qp] / _carbon_density[_qp]) > ln_trans_conc
              ? _test[_i][_qp] *
                    (_u[_qp] / _carbon_density[_qp] -
                     pow(_neighbor_value[_qp] / exp(_A[_qp] + _B[_qp] / _temperature[_qp]),
                         _temperature[_qp] / (_D[_qp] * _temperature[_qp] + _E[_qp])))
              : _test[_i][_qp] * (-_neighbor_value[_qp] +
                                  _u[_qp] / _carbon_density[_qp] *
                                      exp((_A[_qp] + _B[_qp] / _temperature[_qp]) +
                                          (_D[_qp] - 1 + _E[_qp] / _temperature[_qp]) *
                                              (_d1[_qp] - _d2[_qp] * _temperature[_qp])));
      subresidual =
          _test[_i][_qp] * -_diff_neighbor[_qp] * _grad_neighbor_value[_qp] * _normals[_qp];
      break;
    case Moose::Neighbor:
      r = log(_u[_qp] / _carbon_density[_qp]) > ln_trans_conc
              ? -_test[_i][_qp] *
                    (_u[_qp] / _carbon_density[_qp] -
                     pow(_neighbor_value[_qp] / exp(_A[_qp] + _B[_qp] / _temperature[_qp]),
                         _temperature[_qp] / (_D[_qp] * _temperature[_qp] + _E[_qp])))
              : -_test[_i][_qp] * (-_neighbor_value[_qp] +
                                   _u[_qp] / _carbon_density[_qp] *
                                       exp((_A[_qp] + _B[_qp] / _temperature[_qp]) +
                                           (_D[_qp] - 1 + _E[_qp] / _temperature[_qp]) *
                                               (_d1[_qp] - _d2[_qp] * _temperature[_qp])));
      subresidual = _test_neighbor[_i][_qp] * -_diff[_qp] * _grad_u[_qp] * _normals[_qp];
      break;
  }
  return r; // + subresidual;
}
