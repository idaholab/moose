//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHystereticCapillaryPressure.h"
#include "PorousFlowCapillaryPressure.h"

registerMooseObject("PorousFlowApp", PorousFlowHystereticCapillaryPressure);

InputParameters
PorousFlowHystereticCapillaryPressure::validParams()
{
  InputParameters params = PorousFlowVariableBase::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "alpha_d",
      "alpha_d > 0",
      "van Genuchten alpha parameter for the primary drying curve.  If using standard units, this "
      "is measured in Pa^-1.  Suggested value is around 1E-5");
  params.addRequiredRangeCheckedParam<Real>(
      "alpha_w",
      "alpha_w > 0",
      "van Genuchten alpha parameter for the primary wetting curve.  If using standard units, this "
      "is measured in Pa^-1.  Suggested value is around 1E-5");
  params.addRequiredRangeCheckedParam<Real>("n_d",
                                            "n_d > 1",
                                            "van Genuchten n parameter for the primary drying "
                                            "curve.  Dimensionless.  Suggested value is around 2");
  params.addRequiredRangeCheckedParam<Real>("n_w",
                                            "n_w > 1",
                                            "van Genuchten n parameter for the primary wetting "
                                            "curve.  Dimensionless.  Suggested value is around 2");
  params.addRequiredRangeCheckedParam<Real>(
      "S_l_min",
      "S_l_min >= 0 & S_l_min < 1",
      "Minimum liquid saturation for which the van Genuchten expression is valid.  If no lower "
      "extension is used then Pc = Pc_max for liquid saturation <= S_l_min");
  params.addRangeCheckedParam<Real>(
      "S_lr",
      0.0,
      "S_lr >= 0 & S_lr < 1",
      "Liquid residual saturation where the liquid relative permeability is zero.  This is used in "
      "the Land expression to find S_gr_del.  Almost definitely you need to set S_lr equal to the "
      "quantity used for your relative-permeability curves.  Almost definitely you should set S_lr "
      "> S_l_min");
  params.addRequiredRangeCheckedParam<Real>(
      "S_gr_max",
      "S_gr_max >= 0",
      "Residual gas saturation.  1 - S_gr_max is the maximum saturation for which the "
      "van Genuchten expression is valid for the wetting curve.  You must ensure S_gr_max < 1 - "
      "S_l_min.  Often S_gr_max = -0.3136 * ln(porosity) - 0.1334 is used");
  params.addRangeCheckedParam<Real>(
      "Pc_max",
      std::numeric_limits<Real>::max(),
      "Pc_max > 0",
      "Value of capillary pressure at which the lower extension commences.  The default value "
      "means capillary pressure uses the van Genuchten expression for S > S_l_min and is "
      "'infinity' for S <= S_l_min.  This will result in poor convergence around S = S_l_min");
  MooseEnum low_ext_enum("none quadratic exponential", "exponential");
  params.addParam<MooseEnum>(
      "low_extension_type",
      low_ext_enum,
      "Type of extension to use for small liquid saturation values.  The extensions modify the "
      "capillary pressure for all saturation values less than S(Pc_max).  That is, if the "
      "van Genuchten "
      "expression would produce Pc > Pc_max, then the extension is used instead.  NONE: Simply "
      "cut-off the capillary-pressure at Pc_max, so that Pc <= Pc_max for all S.  QUADRATIC: Pc is "
      "a quadratic in S that is continuous and differentiable at S(Pc_max) and has zero derivative "
      "at S = 0 (hence, its value at S = 0 will be greater than Pc_max).  EXPONENTIAL: Pc is an "
      "exponential in S that is continuous and differentiable at S(Pc_max) (hence, its value at S "
      "= 0 will be much greater than Pc_max");
  params.addRangeCheckedParam<Real>(
      "high_ratio",
      0.9,
      "high_ratio > 0 & high_ratio < 1",
      "The extension to the wetting curves commences at high_ratio * (1 - S_gr_del), where 1 - "
      "S_gr_del is the value of the liquid saturation when Pc = 0 (on the wetting curve)");
  MooseEnum high_ext_enum("none power", "power");
  params.addParam<MooseEnum>(
      "high_extension_type",
      high_ext_enum,
      "Type of extension to use for the wetting curves when the liquid saturation is around 1.  "
      "The extensions modify the wetting capillary pressure for all saturation values greater than "
      "high_ratio * (1 - S_gr_del), where 1 - S_gr_del is the value of liquid saturation when the "
      "van Genuchten expression gives Pc = 0.  NONE: use the van Genuchten expression and when S > "
      "1 - S_gr_del, set Pc = 0.  POWER: Pc is proportional to (1 - S)^power, where the "
      "coefficient of proportionality and the power are chosen so the resulting curve is "
      "continuous and differentiable");
  params.addClassDescription(
      "This Material computes information that is required for the computation of hysteretic "
      "capillary pressures in single and multi phase situations");
  return params;
}

PorousFlowHystereticCapillaryPressure::PorousFlowHystereticCapillaryPressure(
    const InputParameters & parameters)
  : PorousFlowVariableBase(parameters),
    _alpha_d(getParam<Real>("alpha_d")),
    _alpha_w(getParam<Real>("alpha_w")),
    _n_d(getParam<Real>("n_d")),
    _n_w(getParam<Real>("n_w")),
    _s_l_min(getParam<Real>("S_l_min")),
    _s_lr(getParam<Real>("S_lr")),
    _s_gr_max(getParam<Real>("S_gr_max")),
    _pc_max(getParam<Real>("Pc_max")),
    _high_ratio(getParam<Real>("high_ratio")),
    _low_ext_type(
        getParam<MooseEnum>("low_extension_type")
            .getEnum<PorousFlowVanGenuchten::LowCapillaryPressureExtension::ExtensionStrategy>()),
    _s_low_d(PorousFlowVanGenuchten::saturationHys(_pc_max, _s_l_min, 0.0, _alpha_d, _n_d)),
    _dpc_low_d(
        PorousFlowVanGenuchten::dcapillaryPressureHys(_s_low_d, _s_l_min, 0.0, _alpha_d, _n_d)),
    _low_ext_d(_low_ext_type, _s_low_d, _pc_max, _dpc_low_d),
    _s_low_w(PorousFlowVanGenuchten::saturationHys(_pc_max, _s_l_min, _s_gr_max, _alpha_w, _n_w)),
    _dpc_low_w(PorousFlowVanGenuchten::dcapillaryPressureHys(
        _s_low_w, _s_l_min, _s_gr_max, _alpha_w, _n_w)),
    _low_ext_w(_low_ext_type, _s_low_w, _pc_max, _dpc_low_w),
    _high_ext_type(
        getParam<MooseEnum>("high_extension_type")
            .getEnum<PorousFlowVanGenuchten::HighCapillaryPressureExtension::ExtensionStrategy>()),
    _s_high(_high_ratio * (1 - _s_gr_max)),
    _pc_high(
        PorousFlowVanGenuchten::capillaryPressureHys(_s_high, _s_l_min, _s_gr_max, _alpha_w, _n_w)),
    _dpc_high(PorousFlowVanGenuchten::dcapillaryPressureHys(
        _s_high, _s_l_min, _s_gr_max, _alpha_w, _n_w)),
    _high_ext(_high_ext_type, _s_high, _pc_high, _dpc_high),
    _hys_order(_nodal_material ? getMaterialProperty<unsigned>("PorousFlow_hysteresis_order_nodal")
                               : getMaterialProperty<unsigned>("PorousFlow_hysteresis_order_qp")),
    _hys_order_old(_nodal_material
                       ? getMaterialPropertyOld<unsigned>("PorousFlow_hysteresis_order_nodal")
                       : getMaterialPropertyOld<unsigned>("PorousFlow_hysteresis_order_qp")),
    _hys_sat_tps(
        _nodal_material
            ? getMaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                  "PorousFlow_hysteresis_saturation_tps_nodal")
            : getMaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                  "PorousFlow_hysteresis_saturation_tps_qp")),
    _pc_older(_nodal_material
                  ? getMaterialPropertyOlder<Real>("PorousFlow_hysteretic_capillary_pressure_nodal")
                  : getMaterialPropertyOlder<Real>("PorousFlow_hysteretic_capillary_pressure_qp")),
    _pc_tps(_nodal_material
                ? declareProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                      "PorousFlow_hysteresis_Pc_tps_nodal")
                : declareProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                      "PorousFlow_hysteresis_Pc_tps_qp")),
    _s_d_tps(_nodal_material
                 ? declareProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                       "PorousFlow_hysteresis_s_d_tps_nodal")
                 : declareProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                       "PorousFlow_hysteresis_s_d_tps_qp")),
    _s_gr_tps(_nodal_material
                  ? declareProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                        "PorousFlow_hysteresis_s_gr_tps_nodal")
                  : declareProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                        "PorousFlow_hysteresis_s_gr_tps_qp")),
    _w_low_ext_tps(
        _nodal_material
            ? declareProperty<std::array<PorousFlowVanGenuchten::LowCapillaryPressureExtension,
                                         PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                  "PorousFlow_hysteresis_w_low_ext_tps_nodal")
            : declareProperty<std::array<PorousFlowVanGenuchten::LowCapillaryPressureExtension,
                                         PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                  "PorousFlow_hysteresis_w_low_ext_tps_qp")),
    _w_high_ext_tps(
        _nodal_material
            ? declareProperty<std::array<PorousFlowVanGenuchten::HighCapillaryPressureExtension,
                                         PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                  "PorousFlow_hysteresis_w_high_ext_tps_nodal")
            : declareProperty<std::array<PorousFlowVanGenuchten::HighCapillaryPressureExtension,
                                         PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                  "PorousFlow_hysteresis_w_high_ext_tps_qp")),
    _s_w_tps(_nodal_material
                 ? declareProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                       "PorousFlow_hysteresis_s_w_tps_nodal")
                 : declareProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                       "PorousFlow_hysteresis_s_w_tps_qp"))
{
  if (_s_gr_max >= 1 - _s_l_min)
    paramError("S_gr_max", "Must be less than 1 - S_l_min");
  if (_s_high < _s_low_w)
    paramError("high_ratio",
               "Should be chosen sufficiently close to 1 so that the high extension does not "
               "interfere with the low extension.  Instead, you may have chosen Pc_max too low");
  if (_s_lr <= _s_l_min)
    mooseWarning("S_lr should usually be greater than S_l_min");
}

void
PorousFlowHystereticCapillaryPressure::initQpStatefulProperties()
{
  PorousFlowVariableBase::initQpStatefulProperties();
  Real tp_sat;
  Real tp_pc;
  if (_hys_order[_qp] >= 1)
  {
    tp_sat = _hys_sat_tps[_qp].at(0);
    tp_pc = PorousFlowVanGenuchten::capillaryPressureHys(
        tp_sat, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
    computeTurningPointInfo(0, tp_sat, tp_pc);
  }
  if (_hys_order[_qp] >= 2)
  {
    tp_sat = _hys_sat_tps[_qp].at(1);
    tp_pc = firstOrderWettingPc(tp_sat);
    computeTurningPointInfo(1, tp_sat, tp_pc);
  }
  if (_hys_order[_qp] >= 3)
  {
    tp_sat = _hys_sat_tps[_qp].at(2);
    tp_pc = secondOrderDryingPc(tp_sat);
    computeTurningPointInfo(2, tp_sat, tp_pc);
  }
}

void
PorousFlowHystereticCapillaryPressure::computeQpProperties()
{
  // size stuff correctly and prepare the derivative matrices with zeroes
  PorousFlowVariableBase::computeQpProperties();

  if (_hys_order[_qp] != _hys_order_old[_qp] && _hys_order[_qp] > 0)
  {
    const unsigned tp_num = _hys_order[_qp] - 1;
    computeTurningPointInfo(tp_num, _hys_sat_tps[_qp].at(tp_num), _pc_older[_qp]);
  }
}

Real
PorousFlowHystereticCapillaryPressure::landSat(Real slDel) const
{
  const Real a = 1.0 / _s_gr_max - 1.0 / (1.0 - _s_lr);
  return (1.0 - slDel) / (1.0 + a * (1.0 - slDel));
}

void
PorousFlowHystereticCapillaryPressure::computeTurningPointInfo(unsigned tp_num,
                                                               Real tp_sat,
                                                               Real tp_pc)
{
  _pc_tps[_qp].at(tp_num) = tp_pc;

  // Quantities on the drying curve:
  // pc on the drying curve at the turning point saturation
  const Real pc_d_tps = PorousFlowVanGenuchten::capillaryPressureHys(
      tp_sat, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  // saturation on the drying curve, at tp_pc
  _s_d_tps[_qp].at(tp_num) =
      PorousFlowVanGenuchten::saturationHys(tp_pc, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);

  // Quantities relevant to the wetting curve defined by the Land expression.
  // s_gr_tps is the Land expression as a function of the turning point saturation
  _s_gr_tps[_qp].at(tp_num) = landSat(tp_sat);
  // the low extension of the wetting curve defined by _s_gr_tps
  const Real s_w_low_ext = PorousFlowVanGenuchten::saturationHys(
      _pc_max, _s_l_min, _s_gr_tps[_qp].at(tp_num), _alpha_w, _n_w);
  const Real dpc_w_low_ext = PorousFlowVanGenuchten::dcapillaryPressureHys(
      s_w_low_ext, _s_l_min, _s_gr_tps[_qp].at(tp_num), _alpha_w, _n_w);
  _w_low_ext_tps[_qp].at(tp_num) = PorousFlowVanGenuchten::LowCapillaryPressureExtension(
      _low_ext_type, s_w_low_ext, _pc_max, dpc_w_low_ext);
  // the high extension of the wetting curve defined by _s_gr_tps
  const Real s_w_high_ext =
      (_high_ext_type == PorousFlowVanGenuchten::HighCapillaryPressureExtension::NONE)
          ? 1.0 - _s_gr_tps[_qp].at(tp_num)
          : _high_ratio *
                (1.0 -
                 _s_gr_tps[_qp].at(
                     tp_num)); // if NONE then use the vanGenuchten all the way to 1 - _s_gr_tps
  const Real pc_w_high_ext =
      PorousFlowVanGenuchten::capillaryPressureHys(s_w_high_ext,
                                                   _s_l_min,
                                                   _s_gr_tps[_qp].at(tp_num),
                                                   _alpha_w,
                                                   _n_w,
                                                   _w_low_ext_tps[_qp].at(tp_num));
  const Real dpc_w_high_ext =
      PorousFlowVanGenuchten::dcapillaryPressureHys(s_w_high_ext,
                                                    _s_l_min,
                                                    _s_gr_tps[_qp].at(tp_num),
                                                    _alpha_w,
                                                    _n_w,
                                                    _w_low_ext_tps[_qp].at(tp_num));
  _w_high_ext_tps[_qp].at(tp_num) = PorousFlowVanGenuchten::HighCapillaryPressureExtension(
      _high_ext_type, s_w_high_ext, pc_w_high_ext, dpc_w_high_ext);

  // saturation on the wetting curve defined by _s_gr_tps, at pc = pc_d_tps
  _s_w_tps[_qp].at(tp_num) = PorousFlowVanGenuchten::saturationHys(pc_d_tps,
                                                                   _s_l_min,
                                                                   _s_gr_tps[_qp].at(tp_num),
                                                                   _alpha_w,
                                                                   _n_w,
                                                                   _w_low_ext_tps[_qp].at(tp_num),
                                                                   _w_high_ext_tps[_qp].at(tp_num));
}

Real
PorousFlowHystereticCapillaryPressure::capillaryPressureQp(Real sat) const
{
  Real pc = 0.0;
  if (_hys_order[_qp] == 0) // on primary drying curve
    pc = PorousFlowVanGenuchten::capillaryPressureHys(
        sat, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  else if (_hys_order[_qp] == 1) // first-order wetting
    pc = firstOrderWettingPc(sat);
  else if (_hys_order[_qp] == 2) // second-order drying
    pc = secondOrderDryingPc(sat);
  else // third order drying and wetting
  {
    const Real tp1 = _hys_sat_tps[_qp].at(1);
    const Real tp2 = _hys_sat_tps[_qp].at(2);
    const Real pc1 = firstOrderWettingPc(sat);
    const Real pc2 = secondOrderDryingPc(sat);
    // handle cases that occur just at the transition from 3rd to 2nd order, or 3rd to 1st order
    if (sat < tp2)
      pc = pc2;
    else if (sat > tp1)
      pc = pc1;
    else if (pc1 >= pc2)
      pc = pc1;
    else if (pc1 <= 0.0 || pc2 <= 0.0)
      pc = 0.0;
    else
      pc = std::exp(std::log(pc1) + (sat - tp1) * (std::log(pc2) - std::log(pc1)) / (tp2 - tp1));
  }
  return pc;
}

Real
PorousFlowHystereticCapillaryPressure::dcapillaryPressureQp(Real sat) const
{
  Real dpc = 0.0;
  if (_hys_order[_qp] == 0) // on primary drying curve
    dpc = PorousFlowVanGenuchten::dcapillaryPressureHys(
        sat, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  else if (_hys_order[_qp] == 1) // first-order wetting
    dpc = dfirstOrderWettingPc(sat);
  else if (_hys_order[_qp] == 2) // second-order drying
    dpc = dsecondOrderDryingPc(sat);
  else // third order drying and wetting
  {
    const Real tp1 = _hys_sat_tps[_qp].at(1);
    const Real tp2 = _hys_sat_tps[_qp].at(2);
    const Real pc1 = firstOrderWettingPc(sat);
    const Real pc2 = secondOrderDryingPc(sat);
    // handle cases that occur just at the transition from 3rd to 2nd order, or 3rd to 1st order
    if (sat < tp2)
      dpc = dsecondOrderDryingPc(sat);
    else if (sat > tp1)
      dpc = dfirstOrderWettingPc(sat);
    else if (pc1 >= pc2)
      dpc = dfirstOrderWettingPc(sat);
    else if (pc1 <= 0.0 || pc2 <= 0.0)
      dpc = 0.0;
    else
    {
      const Real pc =
          std::exp(std::log(pc1) + (sat - tp1) * (std::log(pc2) - std::log(pc1)) / (tp2 - tp1));
      const Real dpc1 = dfirstOrderWettingPc(sat);
      const Real dpc2 = dsecondOrderDryingPc(sat);
      dpc = pc * (dpc1 / pc1 + (std::log(pc2) - std::log(pc1)) / (tp2 - tp1) +
                  (sat - tp1) * (dpc2 / pc2 - dpc1 / pc1) / (tp2 - tp1));
    }
  }
  return dpc;
}

Real
PorousFlowHystereticCapillaryPressure::d2capillaryPressureQp(Real sat) const
{
  Real d2pc = 0.0;
  if (_hys_order[_qp] == 0) // on primary drying curve
    d2pc = PorousFlowVanGenuchten::d2capillaryPressureHys(
        sat, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  else if (_hys_order[_qp] == 1) // first-order wetting
    d2pc = d2firstOrderWettingPc(sat);
  else if (_hys_order[_qp] == 2) // second-order drying
    d2pc = d2secondOrderDryingPc(sat);
  else // third order drying and wetting
  {
    const Real tp1 = _hys_sat_tps[_qp].at(1);
    const Real tp2 = _hys_sat_tps[_qp].at(2);
    const Real pc1 = firstOrderWettingPc(sat);
    const Real pc2 = secondOrderDryingPc(sat);
    // handle cases that occur just at the transition from 3rd to 2nd order, or 3rd to 1st order
    if (sat < tp2)
      d2pc = d2secondOrderDryingPc(sat);
    else if (sat > tp1)
      d2pc = d2firstOrderWettingPc(sat);
    else if (pc1 >= pc2)
      d2pc = d2firstOrderWettingPc(sat);
    else if (pc1 <= 0.0 || pc2 <= 0.0)
      d2pc = 0.0;
    else
    {
      const Real pc =
          std::exp(std::log(pc1) + (sat - tp1) * (std::log(pc2) - std::log(pc1)) / (tp2 - tp1));
      const Real dpc1 = dfirstOrderWettingPc(sat);
      const Real dpc2 = dsecondOrderDryingPc(sat);
      const Real dpc = pc * (dpc1 / pc1 + (std::log(pc2) - std::log(pc1)) / (tp2 - tp1) +
                             (sat - tp1) * (dpc2 / pc2 - dpc1 / pc1) / (tp2 - tp1));
      const Real d2pc1 = d2firstOrderWettingPc(sat);
      const Real d2pc2 = d2secondOrderDryingPc(sat);
      d2pc =
          dpc * (dpc1 / pc1 + (std::log(pc2) - std::log(pc1)) / (tp2 - tp1) +
                 (sat - tp1) * (dpc2 / pc2 - dpc1 / pc1) / (tp2 - tp1)) +
          pc *
              (d2pc1 / pc1 - dpc1 * dpc1 / pc1 / pc1 + (dpc2 / pc2 - dpc1 / pc1) / (tp2 - tp1) +
               (dpc2 / pc2 - dpc1 / pc1) / (tp2 - tp1) +
               (sat - tp1) *
                   (d2pc2 / pc2 - dpc2 * dpc2 / pc2 / pc2 - d2pc1 / pc1 + dpc1 * dpc1 / pc1 / pc1) /
                   (tp2 - tp1));
    }
  }
  return d2pc;
}

Real
PorousFlowHystereticCapillaryPressure::firstOrderWettingPc(Real sat) const
{
  // Simplest version is to just use the wetting curve defined by _s_gr_tps[0], but want
  // continuity at sat = _hys_sat_tps[0] (the turning point), so use the following process. The
  // wetting curve is defined for S <= max_s, where
  const Real max_s = (_w_high_ext_tps[_qp].at(0).strategy ==
                      PorousFlowVanGenuchten::HighCapillaryPressureExtension::POWER)
                         ? 1.0
                         : 1.0 - _s_gr_tps[_qp].at(0);
  // define an interpolation: s_to_use smoothly transitions from _s_w_tps[0] (the value of liquid
  // saturation on the wetting curve defined by _s_gr_tps[0]) when sat = _hys_sat_tps[0], to max_s
  // when sat = max_s
  const Real sat_to_use = _s_w_tps[_qp].at(0) + (max_s - _s_w_tps[_qp].at(0)) *
                                                    (sat - _hys_sat_tps[_qp].at(0)) /
                                                    (max_s - _hys_sat_tps[_qp].at(0));
  return PorousFlowVanGenuchten::capillaryPressureHys(sat_to_use,
                                                      _s_l_min,
                                                      _s_gr_tps[_qp].at(0),
                                                      _alpha_w,
                                                      _n_w,
                                                      _w_low_ext_tps[_qp].at(0),
                                                      _w_high_ext_tps[_qp].at(0));
}

Real
PorousFlowHystereticCapillaryPressure::dfirstOrderWettingPc(Real sat) const
{
  const Real max_s = (_w_high_ext_tps[_qp].at(0).strategy ==
                      PorousFlowVanGenuchten::HighCapillaryPressureExtension::POWER)
                         ? 1.0
                         : 1.0 - _s_gr_tps[_qp].at(0);
  const Real sat_to_use = _s_w_tps[_qp].at(0) + (max_s - _s_w_tps[_qp].at(0)) *
                                                    (sat - _hys_sat_tps[_qp].at(0)) /
                                                    (max_s - _hys_sat_tps[_qp].at(0));
  const Real dsat_to_use = (max_s - _s_w_tps[_qp].at(0)) / (max_s - _hys_sat_tps[_qp].at(0));
  return PorousFlowVanGenuchten::dcapillaryPressureHys(sat_to_use,
                                                       _s_l_min,
                                                       _s_gr_tps[_qp].at(0),
                                                       _alpha_w,
                                                       _n_w,
                                                       _w_low_ext_tps[_qp].at(0),
                                                       _w_high_ext_tps[_qp].at(0)) *
         dsat_to_use;
}

Real
PorousFlowHystereticCapillaryPressure::d2firstOrderWettingPc(Real sat) const
{
  const Real max_s = (_w_high_ext_tps[_qp].at(0).strategy ==
                      PorousFlowVanGenuchten::HighCapillaryPressureExtension::POWER)
                         ? 1.0
                         : 1.0 - _s_gr_tps[_qp].at(0);
  const Real sat_to_use = _s_w_tps[_qp].at(0) + (max_s - _s_w_tps[_qp].at(0)) *
                                                    (sat - _hys_sat_tps[_qp].at(0)) /
                                                    (max_s - _hys_sat_tps[_qp].at(0));
  const Real dsat_to_use = (max_s - _s_w_tps[_qp].at(0)) / (max_s - _hys_sat_tps[_qp].at(0));
  return PorousFlowVanGenuchten::d2capillaryPressureHys(sat_to_use,
                                                        _s_l_min,
                                                        _s_gr_tps[_qp].at(0),
                                                        _alpha_w,
                                                        _n_w,
                                                        _w_low_ext_tps[_qp].at(0),
                                                        _w_high_ext_tps[_qp].at(0)) *
         dsat_to_use * dsat_to_use;
}

Real
PorousFlowHystereticCapillaryPressure::secondOrderDryingPc(Real sat) const
{
  // Simplest version is to just use the primary drying curve, but want
  // continuity at sat = _hys_sat_tps[0] (the dry-to-wet turning point) and sat = _hys_sat_tps[1]
  // (the wet-to-dry turning point), so use the following process.
  const Real tp0 = _hys_sat_tps[_qp].at(0);
  const Real tp1 = _hys_sat_tps[_qp].at(1);
  const Real s1 = _s_d_tps[_qp].at(1);
  const Real sat_to_use =
      (sat >= tp0) ? tp0 + (sat - tp0) * (s1 - tp0) / (tp1 - tp0)
                   : sat; // final case can occur just at transition from 2nd to 0th order
  return PorousFlowVanGenuchten::capillaryPressureHys(
      sat_to_use, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
}

Real
PorousFlowHystereticCapillaryPressure::dsecondOrderDryingPc(Real sat) const
{
  const Real tp0 = _hys_sat_tps[_qp].at(0);
  const Real tp1 = _hys_sat_tps[_qp].at(1);
  const Real s1 = _s_d_tps[_qp].at(1);
  const Real sat_to_use = (sat >= tp0) ? tp0 + (sat - tp0) * (s1 - tp0) / (tp1 - tp0) : sat;
  const Real dsat_to_use = (sat >= tp0) ? (s1 - tp0) / (tp1 - tp0) : 1.0;
  return PorousFlowVanGenuchten::dcapillaryPressureHys(
             sat_to_use, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d) *
         dsat_to_use;
}

Real
PorousFlowHystereticCapillaryPressure::d2secondOrderDryingPc(Real sat) const
{
  const Real tp0 = _hys_sat_tps[_qp].at(0);
  const Real tp1 = _hys_sat_tps[_qp].at(1);
  const Real s1 = _s_d_tps[_qp].at(1);
  const Real sat_to_use = (sat >= tp0) ? tp0 + (sat - tp0) * (s1 - tp0) / (tp1 - tp0) : sat;
  const Real dsat_to_use = (sat >= tp0) ? (s1 - tp0) / (tp1 - tp0) : 1.0;
  return PorousFlowVanGenuchten::d2capillaryPressureHys(
             sat_to_use, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d) *
         dsat_to_use * dsat_to_use;
}

Real
PorousFlowHystereticCapillaryPressure::firstOrderWettingSat(Real pc) const
{
  // this is inverse of firstOrderWettingPc: see that method for comments
  const Real sat_to_use = PorousFlowVanGenuchten::saturationHys(pc,
                                                                _s_l_min,
                                                                _s_gr_tps[_qp].at(0),
                                                                _alpha_w,
                                                                _n_w,
                                                                _w_low_ext_tps[_qp].at(0),
                                                                _w_high_ext_tps[_qp].at(0));
  const Real max_s = (_w_high_ext_tps[_qp].at(0).strategy ==
                      PorousFlowVanGenuchten::HighCapillaryPressureExtension::POWER)
                         ? 1.0
                         : 1.0 - _s_gr_tps[_qp].at(0);
  if (sat_to_use > max_s) // this occurs when using no high extension and pc = 0
    return max_s;
  return (sat_to_use - _s_w_tps[_qp].at(0)) * (max_s - _hys_sat_tps[_qp].at(0)) /
             (max_s - _s_w_tps[_qp].at(0)) +
         _hys_sat_tps[_qp].at(0);
}

Real
PorousFlowHystereticCapillaryPressure::dfirstOrderWettingSat(Real pc) const
{
  const Real dsat_to_use = PorousFlowVanGenuchten::dsaturationHys(pc,
                                                                  _s_l_min,
                                                                  _s_gr_tps[_qp].at(0),
                                                                  _alpha_w,
                                                                  _n_w,
                                                                  _w_low_ext_tps[_qp].at(0),
                                                                  _w_high_ext_tps[_qp].at(0));
  const Real max_s = (_w_high_ext_tps[_qp].at(0).strategy ==
                      PorousFlowVanGenuchten::HighCapillaryPressureExtension::POWER)
                         ? 1.0
                         : 1.0 - _s_gr_tps[_qp].at(0);
  if (pc <= 0.0 && _w_high_ext_tps[_qp].at(0).strategy ==
                       PorousFlowVanGenuchten::HighCapillaryPressureExtension::NONE)
    return 0.0;
  return dsat_to_use * (max_s - _hys_sat_tps[_qp].at(0)) / (max_s - _s_w_tps[_qp].at(0));
}

Real
PorousFlowHystereticCapillaryPressure::d2firstOrderWettingSat(Real pc) const
{
  const Real d2sat_to_use = PorousFlowVanGenuchten::d2saturationHys(pc,
                                                                    _s_l_min,
                                                                    _s_gr_tps[_qp].at(0),
                                                                    _alpha_w,
                                                                    _n_w,
                                                                    _w_low_ext_tps[_qp].at(0),
                                                                    _w_high_ext_tps[_qp].at(0));
  const Real max_s = (_w_high_ext_tps[_qp].at(0).strategy ==
                      PorousFlowVanGenuchten::HighCapillaryPressureExtension::POWER)
                         ? 1.0
                         : 1.0 - _s_gr_tps[_qp].at(0);
  if (pc <= 0.0 && _w_high_ext_tps[_qp].at(0).strategy ==
                       PorousFlowVanGenuchten::HighCapillaryPressureExtension::NONE)
    return 0.0;
  return d2sat_to_use * (max_s - _hys_sat_tps[_qp].at(0)) / (max_s - _s_w_tps[_qp].at(0));
}

Real
PorousFlowHystereticCapillaryPressure::secondOrderDryingSat(Real pc) const
{
  // This is the inverse of secondOrderDryingPc: see that method for comments
  const Real sat_to_use =
      PorousFlowVanGenuchten::saturationHys(pc, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  const Real tp0 = _hys_sat_tps[_qp].at(0);
  const Real tp1 = _hys_sat_tps[_qp].at(1);
  const Real s1 = _s_d_tps[_qp].at(1);
  return (sat_to_use >= tp0) ? (sat_to_use - tp0) * (tp1 - tp0) / (s1 - tp0) + tp0 : sat_to_use;
}

Real
PorousFlowHystereticCapillaryPressure::dsecondOrderDryingSat(Real pc) const
{
  const Real sat_to_use =
      PorousFlowVanGenuchten::saturationHys(pc, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  const Real dsat_to_use =
      PorousFlowVanGenuchten::dsaturationHys(pc, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  const Real tp0 = _hys_sat_tps[_qp].at(0);
  const Real tp1 = _hys_sat_tps[_qp].at(1);
  const Real s1 = _s_d_tps[_qp].at(1);
  return (sat_to_use >= tp0) ? dsat_to_use * (tp1 - tp0) / (s1 - tp0) : dsat_to_use;
}

Real
PorousFlowHystereticCapillaryPressure::d2secondOrderDryingSat(Real pc) const
{
  const Real sat_to_use =
      PorousFlowVanGenuchten::saturationHys(pc, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  const Real d2sat_to_use =
      PorousFlowVanGenuchten::d2saturationHys(pc, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  const Real tp0 = _hys_sat_tps[_qp].at(0);
  const Real tp1 = _hys_sat_tps[_qp].at(1);
  const Real s1 = _s_d_tps[_qp].at(1);
  return (sat_to_use >= tp0) ? d2sat_to_use * (tp1 - tp0) / (s1 - tp0) : d2sat_to_use;
}

Real
PorousFlowHystereticCapillaryPressure::liquidSaturationQp(Real pc) const
{
  Real sat = 0.0;
  if (_hys_order[_qp] == 0) // on primary drying curve
    sat = PorousFlowVanGenuchten::saturationHys(pc, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  else if (_hys_order[_qp] == 1) // first-order wetting
    sat = firstOrderWettingSat(pc);
  else if (_hys_order[_qp] == 2) // second-order drying
    sat = secondOrderDryingSat(pc);
  else // third order drying and wetting
  {
    // NOTE: this is not the exact inverse of the third-order capillary-pressure formula, but only
    // the approximate inverse.  In any simulation, only liquidSaturationQp or capillaryPressureQp
    // are used (not both) so having a slightly different formulation for these two functions is OK
    // Rationale: when pc is close to pc_tp1 then use the first-order wetting curve; when pc is
    // close to pc_tp2 then use the second-order drying curve
    const Real pc_tp1 = _pc_tps[_qp].at(1); // pc on first-order wetting at TP_1
    const Real pc_tp2 = _pc_tps[_qp].at(2); // pc on second-order drying at TP_2
    const Real sat1 = firstOrderWettingSat(pc);
    const Real sat2 = secondOrderDryingSat(pc);
    // handle cases that occur just at the transition from 3rd to 2nd order, or 3rd to 1st order
    if (pc_tp1 <= 0.0 || pc <= 0.0 ||
        pc_tp2 <= 0.0) // only the first condition is strictly necessary as cannot get pc==0 or
                       // pc_tp2=0 without pc_tp1=0 in reality.  The other conditions are added in
                       // case of numerical strangenesses
      sat = sat2;
    else if (pc > pc_tp2)
      sat = sat2;
    else if (pc < pc_tp1)
      sat = sat1;
    else
      sat = sat1 + (std::log(pc) - std::log(pc_tp1)) * (sat2 - sat1) / std::log(pc_tp2 / pc_tp1);
  }
  return sat;
}

Real
PorousFlowHystereticCapillaryPressure::dliquidSaturationQp(Real pc) const
{
  Real dsat = 0.0;
  if (_hys_order[_qp] == 0) // on primary drying curve
    dsat = PorousFlowVanGenuchten::dsaturationHys(pc, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  else if (_hys_order[_qp] == 1) // first-order wetting
    dsat = dfirstOrderWettingSat(pc);
  else if (_hys_order[_qp] == 2) // second-order drying
    dsat = dsecondOrderDryingSat(pc);
  else // third order drying and wetting
  {
    const Real pc_tp1 = _pc_tps[_qp].at(1); // pc on first-order wetting at TP_1
    const Real pc_tp2 = _pc_tps[_qp].at(2); // pc on second-order drying at TP_2
    const Real sat1 = firstOrderWettingSat(pc);
    const Real sat2 = secondOrderDryingSat(pc);
    const Real dsat1 = dfirstOrderWettingSat(pc);
    const Real dsat2 = dsecondOrderDryingSat(pc);
    if (pc_tp1 <= 0.0 || pc <= 0.0 || pc_tp2 <= 0.0)
      dsat = dsat2;
    else if (pc > pc_tp2)
      dsat = dsat2;
    else if (pc < pc_tp1)
      dsat = dsat1;
    else
      dsat = dsat1 + 1.0 / pc * (sat2 - sat1) / std::log(pc_tp2 / pc_tp1) +
             (std::log(pc) - std::log(pc_tp1)) * (dsat2 - dsat1) / std::log(pc_tp2 / pc_tp1);
  }
  return dsat;
}

Real
PorousFlowHystereticCapillaryPressure::d2liquidSaturationQp(Real pc) const
{
  Real d2sat = 0.0;
  if (_hys_order[_qp] == 0) // on primary drying curve
    d2sat = PorousFlowVanGenuchten::d2saturationHys(pc, _s_l_min, 0.0, _alpha_d, _n_d, _low_ext_d);
  else if (_hys_order[_qp] == 1) // first-order wetting
    d2sat = d2firstOrderWettingSat(pc);
  else if (_hys_order[_qp] == 2) // second-order drying
    d2sat = d2secondOrderDryingSat(pc);
  else // third order drying and wetting
  {
    const Real pc_tp1 = _pc_tps[_qp].at(1); // pc on first-order wetting at TP_1
    const Real pc_tp2 = _pc_tps[_qp].at(2); // pc on second-order drying at TP_2
    const Real sat1 = firstOrderWettingSat(pc);
    const Real sat2 = secondOrderDryingSat(pc);
    const Real dsat1 = dfirstOrderWettingSat(pc);
    const Real dsat2 = dsecondOrderDryingSat(pc);
    const Real d2sat1 = d2firstOrderWettingSat(pc);
    const Real d2sat2 = d2secondOrderDryingSat(pc);
    if (pc_tp1 <= 0.0 || pc <= 0.0 || pc_tp2 <= 0.0)
      d2sat = d2sat2;
    else if (pc > pc_tp2)
      d2sat = d2sat2;
    else if (pc < pc_tp1)
      d2sat = d2sat1;
    else
      d2sat = d2sat1 - 1.0 / pc / pc * (sat2 - sat1) / std::log(pc_tp2 / pc_tp1) +
              2.0 / pc * (dsat2 - dsat1) / std::log(pc_tp2 / pc_tp1) +
              (std::log(pc) - std::log(pc_tp1)) * (d2sat2 - d2sat1) / std::log(pc_tp2 / pc_tp1);
  }
  return d2sat;
}
