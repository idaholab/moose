//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHystereticRelativePermeabilityBase.h"

InputParameters
PorousFlowHystereticRelativePermeabilityBase::validParams()
{
  InputParameters params = PorousFlowMaterialBase::validParams();
  params.addRangeCheckedParam<Real>(
      "S_lr",
      0.0,
      "S_lr >= 0 & S_lr < 1",
      "Liquid residual saturation.  At liquid saturation = S_lr, the liquid relative permeability "
      "is zero and the gas relative permeability is k_rg_max.  Almost definitely you need to set "
      "S_lr equal to the quantity used for your hysteretic capillary-pressure curve, if you are "
      "using one.");
  params.addRequiredRangeCheckedParam<Real>(
      "S_gr_max",
      "S_gr_max >= 0 & S_gr_max < 1",
      "Residual gas saturation.  For liquid saturation = 1 - S_gr_max, the gas wetting "
      "relative-permeability is zero, if the turning point was <= S_lr.  1 - S_gr_max is the "
      "maximum liquid saturation for which the van Genuchten expression is valid for the liquid "
      "relative-permeability wetting curve.  You must ensure S_gr_max < 1 - S_lr.  Often S_gr_max "
      "= -0.3136 * ln(porosity) - 0.1334 is used");
  params.addRequiredRangeCheckedParam<Real>(
      "m", "m > 0 & m < 1", "van Genuchten m parameter.  Suggested value is around 0.9");
  params.addPrivateParam<std::string>("pf_material_type", "relative_permeability");
  params.addPrivateParam<bool>("is_ad", false);
  params.addClassDescription("PorousFlow material that computes relative permeability for 1-phase "
                             "or 2-phase models that include hysteresis");
  return params;
}

PorousFlowHystereticRelativePermeabilityBase::PorousFlowHystereticRelativePermeabilityBase(
    const InputParameters & parameters)
  : PorousFlowMaterialBase(parameters),
    _s_lr(getParam<Real>("S_lr")),
    _s_gr_max(getParam<Real>("S_gr_max")),
    _m(getParam<Real>("m")),
    _saturation(_nodal_material
                    ? getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")
                    : getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp")),
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
    _relative_permeability(
        _nodal_material ? declareProperty<Real>("PorousFlow_relative_permeability_nodal" + _phase)
                        : declareProperty<Real>("PorousFlow_relative_permeability_qp" + _phase)),
    _drelative_permeability_ds(
        _nodal_material
            ? declarePropertyDerivative<Real>("PorousFlow_relative_permeability_nodal" + _phase,
                                              _saturation_variable_name)
            : declarePropertyDerivative<Real>("PorousFlow_relative_permeability_qp" + _phase,
                                              _saturation_variable_name)),
    _s_gr_tp0(_nodal_material
                  ? declareProperty<Real>("PorousFlow_hysteresis_s_gr_tp0_nodal" + _phase)
                  : declareProperty<Real>("PorousFlow_hysteresis_s_gr_tp0_qp" + _phase))
{
  mooseAssert(_dictator.numPhases() <= 2,
              "PorousFlowHystereticRelativePermeability Materials can only be used for "
              "1-phase or 2-phase models.  The Dictator proclaims that the number of phases "
              "exceeds 2, and dictators are never incorrect.");
  if (_s_gr_max >= 1 - _s_lr)
    paramError("S_gr_max", "Must be less than 1 - S_lr");
}

void
PorousFlowHystereticRelativePermeabilityBase::initQpStatefulProperties()
{
  PorousFlowMaterialBase::initQpStatefulProperties();
  if (_hys_order[_qp] >= 1)
    computeTurningPoint0Info(_hys_sat_tps[_qp].at(0));

  computeRelPermQp();
}

void
PorousFlowHystereticRelativePermeabilityBase::computeQpProperties()
{
  PorousFlowMaterialBase::computeQpProperties();

  if (_hys_order[_qp] != _hys_order_old[_qp] && _hys_order[_qp] == 1)
    computeTurningPoint0Info(_hys_sat_tps[_qp].at(0));

  computeRelPermQp();
}

Real
PorousFlowHystereticRelativePermeabilityBase::landSat(Real slDel) const
{
  const Real a = 1.0 / _s_gr_max - 1.0 / (1.0 - _s_lr);
  return (1.0 - slDel) / (1.0 + a * (1.0 - slDel));
}

void
PorousFlowHystereticRelativePermeabilityBase::computeTurningPoint0Info(Real tp_sat)
{
  // s_gr_tp0 is the Land expression as a function of the zeroth turning point saturation
  _s_gr_tp0[_qp] = landSat(tp_sat);
}
