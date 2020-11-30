//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHystereticRelativePermeabilityLiquid.h"
#include "PorousFlowVanGenuchten.h"

registerMooseObject("PorousFlowApp", PorousFlowHystereticRelativePermeabilityLiquid);

InputParameters
PorousFlowHystereticRelativePermeabilityLiquid::validParams()
{
  InputParameters params = PorousFlowHystereticRelativePermeabilityBase::validParams();
  params.addRangeCheckedParam<Real>(
      "liquid_modification_range",
      0.9,
      "liquid_modification_range > 0 & liquid_modification_range <= 1",
      "The wetting liquid relative permeability will be a cubic between S_l = "
      "liquid_modification_range * (1 - S_gr_Del) and S_l = 1.0 - 0.5 * S_gr_Del");
  params.addClassDescription(
      "PorousFlow material that computes relative permeability of the liquid phase in 1-phase or "
      "2-phase models that include hysteresis.  You should ensure that the 'phase' for this "
      "Material does indeed represent the liquid phase");
  return params;
}

PorousFlowHystereticRelativePermeabilityLiquid::PorousFlowHystereticRelativePermeabilityLiquid(
    const InputParameters & parameters)
  : PorousFlowHystereticRelativePermeabilityBase(parameters),
    _liquid_phase(_phase_num),
    _liquid_modification_range(getParam<Real>("liquid_modification_range")),
    _kl_begin(_nodal_material ? declareProperty<Real>("PorousFlow_hysteresis_kr_l_begin_nodal")
                              : declareProperty<Real>("PorousFlow_hysteresis_kr_l_begin_qp")),
    _klp_begin(_nodal_material ? declareProperty<Real>("PorousFlow_hysteresis_krp_l_begin_nodal")
                               : declareProperty<Real>("PorousFlow_hysteresis_krp_l_begin_qp")),
    _kl_end(_nodal_material ? declareProperty<Real>("PorousFlow_hysteresis_kr_l_end_nodal")
                            : declareProperty<Real>("PorousFlow_hysteresis_kr_l_end_qp")),
    _klp_end(_nodal_material ? declareProperty<Real>("PorousFlow_hysteresis_krp_l_end_nodal")
                             : declareProperty<Real>("PorousFlow_hysteresis_krp_l_end_qp"))
{
}

void
PorousFlowHystereticRelativePermeabilityLiquid::computeRelPermQp()
{
  const Real sl = _saturation[_qp][_liquid_phase];

  if (_hys_order[_qp] == 0)
  {
    _relative_permeability[_qp] = PorousFlowVanGenuchten::relativePermeabilityHys(
        sl, _s_lr, 0.0, _s_gr_max, 1.0, _m, _liquid_modification_range, 0.0, 0.0, 0.0, 0.0);
    _drelative_permeability_ds[_qp] = PorousFlowVanGenuchten::drelativePermeabilityHys(
        sl, _s_lr, 0.0, _s_gr_max, 1.0, _m, _liquid_modification_range, 0.0, 0.0, 0.0, 0.0);
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
        PorousFlowVanGenuchten::relativePermeabilityHys(sl,
                                                        _s_lr,
                                                        s_gas_max,
                                                        _s_gr_max,
                                                        effective_liquid_tp,
                                                        _m,
                                                        _liquid_modification_range,
                                                        _kl_begin[_qp],
                                                        _klp_begin[_qp],
                                                        _kl_end[_qp],
                                                        _klp_end[_qp]);
    _drelative_permeability_ds[_qp] =
        PorousFlowVanGenuchten::drelativePermeabilityHys(sl,
                                                         _s_lr,
                                                         s_gas_max,
                                                         _s_gr_max,
                                                         effective_liquid_tp,
                                                         _m,
                                                         _liquid_modification_range,
                                                         _kl_begin[_qp],
                                                         _klp_begin[_qp],
                                                         _kl_end[_qp],
                                                         _klp_end[_qp]);
  }
}

void
PorousFlowHystereticRelativePermeabilityLiquid::computeTurningPoint0Info(Real tp_sat)
{
  PorousFlowHystereticRelativePermeabilityBase::computeTurningPoint0Info(tp_sat);

  // following ternaries deal with the case where the turning-point saturation occurs in the
  // low-saturation region (tp_sat < _s_lr).  There is "no hysteresis along the extension"
  // according to Doughty2008, so assume that the wetting curve is the same as would occur if
  // the turning-point saturation occured at _s_lr
  const Real effective_liquid_tp = (tp_sat < _s_lr) ? _s_lr : tp_sat;
  const Real s_gas_max = (tp_sat < _s_lr) ? _s_gr_max : _s_gr_tp0[_qp];

  // compute and record the wetting-curve value and derivative at s_begin
  const Real s_begin = _liquid_modification_range * (1.0 - s_gas_max);
  _kl_begin[_qp] = PorousFlowVanGenuchten::relativePermeabilityHys(s_begin,
                                                                   _s_lr,
                                                                   s_gas_max,
                                                                   _s_gr_max,
                                                                   effective_liquid_tp,
                                                                   _m,
                                                                   _liquid_modification_range,
                                                                   0.0,
                                                                   0.0,
                                                                   0.0,
                                                                   0.0);
  _klp_begin[_qp] = PorousFlowVanGenuchten::drelativePermeabilityHys(s_begin,
                                                                     _s_lr,
                                                                     s_gas_max,
                                                                     _s_gr_max,
                                                                     effective_liquid_tp,
                                                                     _m,
                                                                     _liquid_modification_range,
                                                                     0.0,
                                                                     0.0,
                                                                     0.0,
                                                                     0.0);
  // compute and record the drying curve information at s_end (the drying curve is used because
  // s_end = 1.0 - 0.5 * s_gas_max in PorousFlowVanGenuchten::relativePermeabilityHys)
  const Real s_end = 1.0 - 0.5 * s_gas_max;
  _kl_end[_qp] = PorousFlowVanGenuchten::relativePermeabilityHys(s_end,
                                                                 _s_lr,
                                                                 s_gas_max,
                                                                 _s_gr_max,
                                                                 effective_liquid_tp,
                                                                 _m,
                                                                 _liquid_modification_range,
                                                                 0.0,
                                                                 0.0,
                                                                 0.0,
                                                                 0.0);
  _klp_end[_qp] = PorousFlowVanGenuchten::drelativePermeabilityHys(s_end,
                                                                   _s_lr,
                                                                   s_gas_max,
                                                                   _s_gr_max,
                                                                   effective_liquid_tp,
                                                                   _m,
                                                                   _liquid_modification_range,
                                                                   0.0,
                                                                   0.0,
                                                                   0.0,
                                                                   0.0);
}
