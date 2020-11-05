//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHysteresisOrder.h"
#include "Conversion.h"

registerMooseObject("PorousFlowApp", PorousFlowHysteresisOrder);

InputParameters
PorousFlowHysteresisOrder::validParams()
{
  InputParameters params = PorousFlowMaterial::validParams();
  params.addPrivateParam<std::string>("pf_material_type", "hysteresis_order");
  params.addParam<unsigned>("liquid_phase", 0, "The phase number of the liquid phase");
  params.addParam<unsigned>(
      "initial_order",
      0,
      "The initial order.  0 means the simulation will begin on the primary drying curve.  1 means "
      "the simulation will begin on the first-order wetting curve.  2 means the simulation will "
      "begin on the second-order drying curve, etc.");
  params.addParam<std::vector<Real>>(
      "previous_turning_points",
      std::vector<Real>(),
      "The turning points (liquid saturation values) at occured prior to the simulation.  There "
      "must be exactly initial_order of these values.  The first value is the liquid saturation at "
      "the transition from primary-drying to first-order-wetting; the second value is the liquid "
      "saturation at the transition from first-order-wetting to second-order-drying; and so on.  "
      "You must ensure that your initial saturation field is commensurate with the "
      "previous_turning_points.");
  params.addClassDescription("Computes hysteresis order for use in hysteretic capillary pressures "
                             "and relative permeabilities");
  return params;
}

PorousFlowHysteresisOrder::PorousFlowHysteresisOrder(const InputParameters & parameters)
  : DerivativeMaterialInterface<PorousFlowMaterial>(parameters),
    _liquid_ph_num(getParam<unsigned>("liquid_phase")),
    _liquid_phase(Moose::stringify(_liquid_ph_num)),
    _initial_order(getParam<unsigned>("initial_order")),
    _previous_turning_points(getParam<std::vector<Real>>("previous_turning_points")),
    _sat_old(_nodal_material
                 ? getMaterialPropertyOld<std::vector<Real>>("PorousFlow_saturation_nodal")
                 : getMaterialPropertyOld<std::vector<Real>>("PorousFlow_saturation_qp")),
    _sat_older(_nodal_material
                   ? getMaterialPropertyOlder<std::vector<Real>>("PorousFlow_saturation_nodal")
                   : getMaterialPropertyOlder<std::vector<Real>>("PorousFlow_saturation_qp")),
    _hys_order(_nodal_material ? declareProperty<unsigned>("PorousFlow_hysteresis_order_nodal")
                               : declareProperty<unsigned>("PorousFlow_hysteresis_order_qp")),
    _hys_order_old(_nodal_material
                       ? getMaterialPropertyOld<unsigned>("PorousFlow_hysteresis_order_nodal")
                       : getMaterialPropertyOld<unsigned>("PorousFlow_hysteresis_order_qp")),
    _hys_sat_tps(_nodal_material
                     ? declareProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                           "PorousFlow_hysteresis_saturation_tps_nodal")
                     : declareProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                           "PorousFlow_hysteresis_saturation_tps_qp")),
    _hys_sat_tps_old(
        _nodal_material
            ? getMaterialPropertyOld<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                  "PorousFlow_hysteresis_saturation_tps_nodal")
            : getMaterialPropertyOld<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>>(
                  "PorousFlow_hysteresis_saturation_tps_qp"))
{
  if (_liquid_ph_num >= _dictator.numPhases())
    paramError("liquid_phase",
               "The Dictator proclaims that the number of fluid phases is ",
               _dictator.numPhases(),
               " while you have foolishly entered ",
               _liquid_ph_num,
               " for your liquid phase number.  Remember that indexing starts at 0.  Be aware that "
               "the Dictator does not tolerate mistakes.");
  if (_initial_order > PorousFlowConstants::MAX_HYSTERESIS_ORDER)
    paramError("initial_order",
               "The initial order must be less than the max order, which is hard-coded to ",
               PorousFlowConstants::MAX_HYSTERESIS_ORDER);
  if (_previous_turning_points.size() != _initial_order)
    paramError("previous_turning_points",
               "initial_order is ",
               _initial_order,
               " so there must be this many previous_turning_points");
  for (const auto & tp : _previous_turning_points)
    if (tp < 0.0 || tp > 1.0)
      paramError("previous_turning_points",
                 "Each previous_turning_point must lie in the range [0.0, 1.0]");
  // check that the previous_turning_points are ordered correctly
  Real previous_tp = 1.0;
  Real previous_jump = std::numeric_limits<Real>::max();
  bool drying = true;
  for (unsigned i = 0; i < _initial_order; ++i)
  {
    const Real this_jump = _previous_turning_points[i] - previous_tp;
    if (drying)
    {
      if (this_jump >= 0)
        paramError("previous_turning_points",
                   "The previous_turning_points vector, {a, b, c, ...}, must be obey the following "
                   "order: a < 1; b > a; a < c < b, etc");
    }
    else
    {
      if (this_jump <= 0)
        paramError("previous_turning_points",
                   "The previous_turning_points vector, {a, b, c, ...}, must be obey the following "
                   "order: a < 1; b > a; a < c < b, etc");
    }
    if (std::abs(this_jump) >= previous_jump)
      paramError("previous_turning_points",
                 "The previous_turning_points vector, {a, b, c, ...}, must be obey the following "
                 "order: a < 1; b > a; a < c < b, etc");
    drying = !drying;
    previous_tp = _previous_turning_points[i];
    previous_jump = std::abs(this_jump);
  }
}

void
PorousFlowHysteresisOrder::initQpStatefulProperties()
{
  _hys_order[_qp] = _initial_order;
  for (unsigned i = 0; i < _initial_order; ++i)
    _hys_sat_tps[_qp].at(i) = _previous_turning_points[i];
}

void
PorousFlowHysteresisOrder::computeQpProperties()
{
  // NOTE to readers: the computed properties depend on old and older quantities only.  Hence, it
  // would be nice if we could compute them only at the start of each timestep instead of every
  // nonlinear iteration

  // whether saturation has been reducing ("drying" or "draining"):
  const bool drying = (_hys_order_old[_qp] % 2 == 0);
  // whether have been drying but are now increasing saturation ("wetting" or "imbibing"):
  const bool dry2wet = drying && (_sat_old[_qp][_liquid_ph_num] > _sat_older[_qp][_liquid_ph_num]);
  // whether have been wetting but are now decreasing saturation:
  const bool wet2dry = !drying && (_sat_old[_qp][_liquid_ph_num] < _sat_older[_qp][_liquid_ph_num]);
  if ((dry2wet || wet2dry) && _hys_order_old[_qp] < PorousFlowConstants::MAX_HYSTERESIS_ORDER)
  {
    _hys_order[_qp] = _hys_order_old[_qp] + 1;
    _hys_sat_tps[_qp] = _hys_sat_tps_old[_qp];
    _hys_sat_tps[_qp].at(_hys_order_old[_qp]) = _sat_older[_qp][_liquid_ph_num];
  }
  else
  {
    // no change in wetting/drying direction
    _hys_order[_qp] = _hys_order_old[_qp];
    _hys_sat_tps[_qp] = _hys_sat_tps_old[_qp];
  }
  if (_sat_old[_qp][_liquid_ph_num] >= 1.0)
    _hys_order[_qp] = 0; // assume the system gets back to the original drying curve

  // reduce order by 1 if _hys_order = max and saturation exceeds bounds of last turning-point
  if (_hys_order[_qp] == PorousFlowConstants::MAX_HYSTERESIS_ORDER)
  {
    // the maxiumum-order curve is a drying curve, and the saturation exceeds the previous turning
    // point:
    const bool drying_and_high_sat =
        (_hys_order[_qp] % 2 == 0) &&
        (_sat_old[_qp][_liquid_ph_num] >= _hys_sat_tps[_qp].at(_hys_order[_qp] - 1));
    // the maxiumum-order curve is a wetting curve, and the saturation is less than the previous
    // turning point:
    const bool wetting_and_low_sat =
        (_hys_order[_qp] % 2 == 1) &&
        (_sat_old[_qp][_liquid_ph_num] <= _hys_sat_tps[_qp].at(_hys_order[_qp] - 1));
    if (drying_and_high_sat || wetting_and_low_sat)
      _hys_order[_qp] -= 1;
  }

  // reduce order by 2 if saturation exceeds the bounds of the second-to-last turning-point
  // encountered
  bool can_reduce_order = (_hys_order[_qp] > 1);
  while (can_reduce_order)
  {
    can_reduce_order = false;
    // are drying and the saturation is <= a previously-encountered turning point:
    const bool drying_and_low_sat =
        (_hys_order[_qp] % 2 == 0) &&
        (_sat_old[_qp][_liquid_ph_num] <= _hys_sat_tps[_qp].at(_hys_order[_qp] - 2));
    // are wetting and the saturation is >= a previously-encountered turning point:
    const bool wetting_and_high_sat =
        (_hys_order[_qp] % 2 == 1) &&
        (_sat_old[_qp][_liquid_ph_num] >= _hys_sat_tps[_qp].at(_hys_order[_qp] - 2));
    if (drying_and_low_sat || wetting_and_high_sat)
    {
      _hys_order[_qp] -= 2;
      can_reduce_order = (_hys_order[_qp] > 1);
    }
  }
}
