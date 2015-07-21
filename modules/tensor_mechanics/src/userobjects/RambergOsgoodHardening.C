#include "RambergOsgoodHardening.h"

template<>
InputParameters validParams<RambergOsgoodHardening>()
{
  InputParameters params = validParams<TensorMechanicsHardeningModel>();
  params.addRequiredParam<Real>("yield_stress", "The yield stress value");
  params.addRequiredParam<Real>("hardening_exponent", "The hardening exponent value");
  params.addRequiredParam<Real>("reference_plastic_strain", "Reference plastic strain value");
  params.addClassDescription("User object for Ramberg-Osgood hardening model");

  return params;
}

RambergOsgoodHardening::RambergOsgoodHardening(const InputParameters & parameters) :
    TensorMechanicsHardeningModel(parameters),
    _yield_stress(getParam<Real>("yield_stress")),
    _hardening_exponent(getParam<Real>("hardening_exponent")),
    _ref_plastic_strain(getParam<Real>("reference_plastic_strain"))
{
}

Real
RambergOsgoodHardening::value(const Real & intnl) const
{
  return _yield_stress * std::pow(intnl/ _ref_plastic_strain + 1, _hardening_exponent);
}

Real
RambergOsgoodHardening::derivative(const Real & intnl) const
{
  return _yield_stress * std::pow(intnl/ _ref_plastic_strain + 1, _hardening_exponent - 1) * _hardening_exponent/ _ref_plastic_strain;
}

// DEPRECATED
RambergOsgoodHardening::RambergOsgoodHardening(const std::string & name, InputParameters parameters) :
    TensorMechanicsHardeningModel(name, parameters),
    _yield_stress(getParam<Real>("yield_stress")),
    _hardening_exponent(getParam<Real>("hardening_exponent")),
    _ref_plastic_strain(getParam<Real>("reference_plastic_strain"))
{
}
