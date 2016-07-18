/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSEntropyError.h"

template<>
InputParameters validParams<NSEntropyError>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredParam<Real>("rho_infty", "Freestream density");
  params.addRequiredParam<Real>("p_infty", "Freestream pressure");
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("pressure", "pressure");
  return params;
}

NSEntropyError::NSEntropyError(const InputParameters & parameters) :
    ElementIntegralPostprocessor(parameters),
    _rho_infty(getParam<Real>("rho_infty")),
    _p_infty(getParam<Real>("p_infty")),
    _gamma(getParam<Real>("gamma")),
    _rho(coupledValue("rho")),
    _pressure(coupledValue("pressure"))
{
}

Real
NSEntropyError::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
NSEntropyError::computeQpIntegral()
{
  Real integrand = (_pressure[_qp] / _p_infty) * std::pow(_rho_infty / _rho[_qp], _gamma) - 1.;
  return integrand * integrand;
}
