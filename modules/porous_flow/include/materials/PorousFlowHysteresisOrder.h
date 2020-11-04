//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterial.h"
#include "DerivativeMaterialInterface.h"
#include "PorousFlowConstants.h"

/**
 * Computes the hysteresis order for use by the hysteretic capillary pressure and
 * relative-permeability objects
 */
class PorousFlowHysteresisOrder : public DerivativeMaterialInterface<PorousFlowMaterial>
{
public:
  static InputParameters validParams();

  PorousFlowHysteresisOrder(const InputParameters & parameters);

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

protected:
  /// Liquid phase number
  const unsigned _liquid_ph_num;

  /// Stringified liquid phase number
  const std::string _liquid_phase;

  /// Initial order
  const unsigned _initial_order;

  /// Previous turning points that were encountered prior to the simulation
  const std::vector<Real> _previous_turning_points;

  /// Old value of saturation
  const MaterialProperty<std::vector<Real>> & _sat_old;

  /// Older value of saturation
  const MaterialProperty<std::vector<Real>> & _sat_older;

  /// Computed hysteresis order at the nodes or quadpoints
  MaterialProperty<unsigned> & _hys_order;

  /// Old value of hysteresis order at the nodes or quadpoints
  const MaterialProperty<unsigned> & _hys_order_old;

  /// Recorded saturation values at the turning points
  MaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>> & _hys_sat_tps;

  /// Old value of recorded saturation values at the turning points
  const MaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>> &
      _hys_sat_tps_old;
};
