#include "TensorMechanicsPlasticTensileExponential.h"

template<>
InputParameters validParams<TensorMechanicsPlasticTensileExponential>()
{
  InputParameters params = validParams<TensorMechanicsPlasticTensile>();
  params.addRequiredRangeCheckedParam<Real>("tensile_strength", "tensile_strength>=0", "Tensile strength");
  params.addParam<Real>("tensile_strength_residual", "Tensile strength at infinite hardening.  If not given, this defaults to tensile_strength, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("tensile_strength_rate", 0, "tensile_strength_rate>=0", "Tensile strength = tensile_strength_residual + (tensile_strength - tensile_strength_residual)*exp(-tensile_strength_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addClassDescription("Associative tensile plasticity with exponential hardening/softening");

  return params;
}

TensorMechanicsPlasticTensileExponential::TensorMechanicsPlasticTensileExponential(const std::string & name,
                                                         InputParameters parameters) :
    TensorMechanicsPlasticTensile(name, parameters),
    _tensile_strength0(getParam<Real>("tensile_strength")),
    _tensile_strength_residual(parameters.isParamValid("tensile_strength_residual") ? getParam<Real>("tensile_strength_residual") : _tensile_strength0),
    _tensile_strength_rate(getParam<Real>("tensile_strength_rate"))
{
}


Real
TensorMechanicsPlasticTensileExponential::tensile_strength(const Real internal_param) const
{
  return _tensile_strength_residual + (_tensile_strength0 - _tensile_strength_residual)*std::exp(-_tensile_strength_rate*internal_param);
}

Real
TensorMechanicsPlasticTensileExponential::dtensile_strength(const Real internal_param) const
{
  return -_tensile_strength_rate*(_tensile_strength0 - _tensile_strength_residual)*std::exp(-_tensile_strength_rate*internal_param);
}
