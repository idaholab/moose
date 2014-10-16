#include "TensorMechanicsPlasticWeakPlaneTensileCubic.h"

template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensileCubic>()
{
  InputParameters params = validParams<TensorMechanicsPlasticWeakPlaneTensile>();
  params.addRequiredRangeCheckedParam<Real>("strength", "strength>=0", "Weak plane tensile strength");
  params.addParam<Real>("strength_residual", "Tensile strength at internal_parameter = limit.  If not given, this defaults to strength, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("strength_limit", 0, "strength_limit>=0", "Tensile strength = cubic between strength (at zero internal parameter), and strength_residual (at internal_parameter = strength_limit).  Set to zero for perfect plasticity");
  params.addClassDescription("Associative weak-plane tensile plasticity with cubic hardening/softening");

  return params;
}

TensorMechanicsPlasticWeakPlaneTensileCubic::TensorMechanicsPlasticWeakPlaneTensileCubic(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticWeakPlaneTensile(name, parameters),
    _tension(getParam<Real>("strength")),
    _tension_residual(parameters.isParamValid("strength_residual") ? getParam<Real>("strength_residual") : _tension),
    _tension_limit(getParam<Real>("strength_limit")),
    _half_tension_limit(0.5*_tension_limit),
    _alpha_tension((_tension - _tension_residual)/4.0/std::pow(_half_tension_limit, 3)),
    _beta_tension(-3.0*_alpha_tension*std::pow(_half_tension_limit, 2))
{
}

Real
TensorMechanicsPlasticWeakPlaneTensileCubic::tensile_strength(const Real internal_param) const
{
  if (internal_param <= 0)
    return _tension;
  else if (internal_param >= _tension_limit)
    return _tension_residual;
  else
    return _alpha_tension*std::pow(internal_param - _half_tension_limit, 3) + _beta_tension*(internal_param - _half_tension_limit) + 0.5*(_tension + _tension_residual);
}

Real
TensorMechanicsPlasticWeakPlaneTensileCubic::dtensile_strength(const Real internal_param) const
{
  if (internal_param <= 0)
    return 0.0;
  else if (internal_param >= _tension_limit)
    return 0.0;
  else
    return 3*_alpha_tension*std::pow(internal_param - _half_tension_limit, 2) + _beta_tension;
}
