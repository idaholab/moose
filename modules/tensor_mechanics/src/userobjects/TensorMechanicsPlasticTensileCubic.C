#include "TensorMechanicsPlasticTensileCubic.h"

template<>
InputParameters validParams<TensorMechanicsPlasticTensileCubic>()
{
  InputParameters params = validParams<TensorMechanicsPlasticTensile>();
  params.addRequiredRangeCheckedParam<Real>("tensile_strength", "tensile_strength>=0", "Tensile strength");
  params.addParam<Real>("tensile_strength_residual", "Tensile strength at infinite hardening.  If not given, this defaults to tensile_strength, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("tensile_strength_limit", 1, "tensile_strength_limit>0", "Tensile strength = tensile_strength at zero hardening, and tensile_strength_residual at internal_parameter = tensile_strength_residual, and is a C1 cubic between these two values");
  params.addClassDescription("Associative tensile plasticity with cubic hardening/softening");

  return params;
}

TensorMechanicsPlasticTensileCubic::TensorMechanicsPlasticTensileCubic(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticTensile(name, parameters),
    _tensile_strength0(getParam<Real>("tensile_strength")),
    _tensile_strength_residual(parameters.isParamValid("tensile_strength_residual") ? getParam<Real>("tensile_strength_residual") : _tensile_strength0),
    _tensile_strength_limit(getParam<Real>("tensile_strength_limit")),
    _half_tensile_strength_limit(0.5*_tensile_strength_limit),
    _alpha_tension((_tensile_strength0 - _tensile_strength_residual)/4.0/std::pow(_half_tensile_strength_limit, 3)),
    _beta_tension(-3.0*_alpha_tension*std::pow(_half_tensile_strength_limit, 2))
{
}


Real
TensorMechanicsPlasticTensileCubic::tensile_strength(const Real internal_param) const
{
  if (internal_param <= 0)
    return _tensile_strength0;
  else if (internal_param >= _tensile_strength_limit)
    return _tensile_strength_residual;
  else
    return _alpha_tension*std::pow(internal_param - _half_tensile_strength_limit, 3) + _beta_tension*(internal_param - _half_tensile_strength_limit) + 0.5*(_tensile_strength0 + _tensile_strength_residual);
}

Real
TensorMechanicsPlasticTensileCubic::dtensile_strength(const Real internal_param) const
{
  if (internal_param <= 0)
    return 0.0;
  else if (internal_param >= _tensile_strength_limit)
    return 0.0;
  else
    return 3*_alpha_tension*std::pow(internal_param - _half_tensile_strength_limit, 2) + _beta_tension;
}
