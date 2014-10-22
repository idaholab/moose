#include "TensorMechanicsPlasticWeakPlaneTensileExponential.h"

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensileExponential>()
{
  InputParameters params = validParams<TensorMechanicsPlasticWeakPlaneTensile>();
  params.addRequiredRangeCheckedParam<Real>("strength", "strength>=0", "Weak plane tensile strength");
  params.addParam<Real>("strength_residual", "Tensile strength at infinite hardening.  If not given, this defaults to strength, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("strength_rate", 0, "strength_rate>=0", "Tensile strength = strength_residual + (strength - strength_residual)*exp(-strength_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addClassDescription("Associative weak-plane tensile plasticity with exponential hardening/softening");

  return params;
}

TensorMechanicsPlasticWeakPlaneTensileExponential::TensorMechanicsPlasticWeakPlaneTensileExponential(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticWeakPlaneTensile(name, parameters),
    _tension_cutoff(getParam<Real>("strength")),
    _tension_cutoff_residual(parameters.isParamValid("strength_residual") ? getParam<Real>("strength_residual") : _tension_cutoff),
    _tension_cutoff_rate(getParam<Real>("strength_rate"))
{
}

Real
TensorMechanicsPlasticWeakPlaneTensileExponential::tensile_strength(const Real internal_param) const
{
  return _tension_cutoff_residual + (_tension_cutoff - _tension_cutoff_residual)*std::exp(-_tension_cutoff_rate*internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneTensileExponential::dtensile_strength(const Real internal_param) const
{
  return -_tension_cutoff_rate*(_tension_cutoff - _tension_cutoff_residual)*std::exp(-_tension_cutoff_rate*internal_param);
}
