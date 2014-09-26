#include "TensorMechanicsPlasticJ2Gaussian.h"

template<>
InputParameters validParams<TensorMechanicsPlasticJ2Gaussian>()
{
  InputParameters params = validParams<TensorMechanicsPlasticJ2>();
  params.addRequiredRangeCheckedParam<Real>("strength", "strength>=0", "Initial value of yield strength before any hardening");
  params.addParam<Real>("strength_residual", 0.0, "Value of yield strength at infinite hardening");
  params.addParam<Real>("rate", 0, "Rate of hardening.  YieldStrength = strength_residual + (strength - strength_residual)*exp(-0.5*(rate*plastic_strain)^2)");
  params.addClassDescription("J2 plasticity, associative, with yieldStrength = strength_residual + (strength - strength_residual)*exp(-0.5*(rate*plastic_strain)^2)");

  return params;
}

TensorMechanicsPlasticJ2Gaussian::TensorMechanicsPlasticJ2Gaussian(const std::string & name,
                                                         InputParameters parameters) :
  TensorMechanicsPlasticJ2(name, parameters),
  _y0(getParam<Real>("strength")),
  _yinf(getParam<Real>("strength_residual")),
  _rate(getParam<Real>("rate"))
{
}

Real
TensorMechanicsPlasticJ2Gaussian::yieldStrength(const Real & intnl) const
{
  return _yinf + (_y0 - _yinf)*std::exp(-0.5*std::pow(_rate*intnl, 2));
}

Real
TensorMechanicsPlasticJ2Gaussian::dyieldStrength(const Real & intnl) const
{
  return -std::pow(_rate, 2)*intnl*(_y0 - _yinf)*std::exp(-0.5*std::pow(_rate*intnl, 2));
}


