//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHystereticRelativePermeabilityGas.h"
#include "PorousFlowVanGenuchten.h"

registerMooseObject("PorousFlowApp", PorousFlowHystereticRelativePermeabilityGas);

InputParameters
PorousFlowHystereticRelativePermeabilityGas::validParams()
{
  InputParameters params = PorousFlowHystereticRelativePermeabilityBase::validParams();
  params.addRangeCheckedParam<Real>(
      "gamma", 0.33, "gamma > 0", "Gamma parameter that is used for the gas relative permeability");
  params.addRangeCheckedParam<Real>(
      "k_rg_max",
      1.0,
      "k_rg_max > 0 & k_rg_max <= 1",
      "Value of the gas relative permeability at liquid saturation = S_lr");
  MooseEnum low_ext_enum("linear_like cubic", "linear_like");
  params.addParam<MooseEnum>(
      "gas_low_extension_type",
      low_ext_enum,
      "Type of extension to use for liquid saturation < S_lr for the gas relative permeability.  "
      "All extensions employ a cubic whose value is 1.0 at liquid saturation = 0, and whose "
      "derivative is zero at liquid saturation = 0, and whose value is k_rg_max at liquid "
      "saturation = S_lr.  linear_like: the derivative at liquid_saturation = S_lr is equal to "
      "(k_rg_max - 1) / S_lr.  cubic: the derivative at liquid_saturation = S_lr equals the "
      "derivative of the unextended drying curve at that point");
  params.addClassDescription(
      "PorousFlow material that computes relative permeability of the gas phase in 1-phase or "
      "2-phase models that include hysteresis.  You should ensure that the 'phase' for this "
      "Material does indeed represent the gas phase");
  return params;
}

PorousFlowHystereticRelativePermeabilityGas::PorousFlowHystereticRelativePermeabilityGas(
    const InputParameters & parameters)
  : PorousFlowHystereticRelativePermeabilityBase(parameters),
    _liquid_phase(_phase_num == 0 ? 1 : 0),
    _gamma(getParam<Real>("gamma")),
    _k_rg_max(getParam<Real>("k_rg_max")),
    _krel_gas_prime((getParam<MooseEnum>("gas_low_extension_type") == "linear_like")
                        ? ((_s_lr > 0) ? (_k_rg_max - 1.0) / _s_lr : 0.0)
                        : PorousFlowVanGenuchten::drelativePermeabilityNWHys(
                              _s_lr, _s_lr, 0.0, _s_gr_max, 1.0, _m, _gamma, _k_rg_max, 0.0))
{
}

void
PorousFlowHystereticRelativePermeabilityGas::computeRelPermQp()
{
  const Real sl = _saturation[_qp][_liquid_phase];

  if (_hys_order[_qp] == 0)
  {
    _relative_permeability[_qp] = PorousFlowVanGenuchten::relativePermeabilityNWHys(
        sl, _s_lr, 0.0, _s_gr_max, 1.0, _m, _gamma, _k_rg_max, _krel_gas_prime);
    // negative in the following from d(liquid_saturation)/d(gas_saturation)
    _drelative_permeability_ds[_qp] = -PorousFlowVanGenuchten::drelativePermeabilityNWHys(
        sl, _s_lr, 0.0, _s_gr_max, 1.0, _m, _gamma, _k_rg_max, _krel_gas_prime);
  }
  else
  {
    // following ternary deals with the case where the turning-point saturation occurs in the
    // low-saturation region (tp_sat < _s_lr).  There is "no hysteresis along the extension"
    // according to Doughty2008, so assume that the wetting curve is the same as would occur if
    // the turning-point saturation occured at _s_lr
    const Real effective_liquid_tp =
        (_hys_sat_tps[_qp].at(0) < _s_lr) ? _s_lr : _hys_sat_tps[_qp].at(0);
    const Real s_gas_max = (_hys_sat_tps[_qp].at(0) < _s_lr) ? _s_gr_max : _s_gr_tp0[_qp];
    _relative_permeability[_qp] =
        PorousFlowVanGenuchten::relativePermeabilityNWHys(sl,
                                                          _s_lr,
                                                          s_gas_max,
                                                          _s_gr_max,
                                                          effective_liquid_tp,
                                                          _m,
                                                          _gamma,
                                                          _k_rg_max,
                                                          _krel_gas_prime);
    // negative in the following from d(liquid_saturation)/d(gas_saturation)
    _drelative_permeability_ds[_qp] =
        -PorousFlowVanGenuchten::drelativePermeabilityNWHys(sl,
                                                            _s_lr,
                                                            s_gas_max,
                                                            _s_gr_max,
                                                            effective_liquid_tp,
                                                            _m,
                                                            _gamma,
                                                            _k_rg_max,
                                                            _krel_gas_prime);
  }
}
