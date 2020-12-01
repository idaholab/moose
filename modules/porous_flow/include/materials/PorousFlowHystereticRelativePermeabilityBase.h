//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialBase.h"
#include "PorousFlowConstants.h"

/**
 * Base material for computing relative permeability for 1-phase and 2-phase hysteretic models
 */
class PorousFlowHystereticRelativePermeabilityBase : public PorousFlowMaterialBase
{
public:
  static InputParameters validParams();

  PorousFlowHystereticRelativePermeabilityBase(const InputParameters & parameters);

protected:
  /// Liquid saturation at which the liquid relperm is zero and the gas relperm is k_rg_max
  const Real _s_lr;

  /// Gas residual saturation
  const Real _s_gr_max;

  /// van Genuchten m parameter
  const Real _m;

  /// Saturation material property
  const MaterialProperty<std::vector<Real>> & _saturation;

  /// Hysteresis order, as computed by PorousFlowHysteresisOrder
  const MaterialProperty<unsigned> & _hys_order;

  /// Old value of hysteresis order, as computed by PorousFlowHysteresisOrder
  const MaterialProperty<unsigned> & _hys_order_old;

  /// Saturation values at the turning points, as computed by PorousFlowHysteresisOrder
  const MaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>> &
      _hys_sat_tps;

  /// Computed relative permeability
  MaterialProperty<Real> & _relative_permeability;

  /// Derivative of relative permeability wrt the saturation of _phase_num (which is not necessarily the liquid phase)
  MaterialProperty<Real> & _drelative_permeability_ds;

  /// Computed nodal or quadpoint values the Land expression, at the turning point from primary drying to first-order wetting
  MaterialProperty<Real> & _s_gr_tp0;

  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Compute the relative permeability and its derivative wrt the _phase_num saturation, at the
   * quadpoints, and store the result in _relative_permeability[_qp] and
   * _drelative_permeability_ds[_qp]
   */
  virtual void computeRelPermQp() = 0;

  /**
   * Compute all relevant quantities at the zeroth turning point (the transition from primary drying
   * to first-order wetting)
   * @param tp_sat Liquid saturation at the turning point
   */
  virtual void computeTurningPoint0Info(Real tp_sat);

private:
  /**
   * @return the value of gas saturation (called S_gr^Delta in the markdown documentation) using the
   * Land expression.  (This is a function of the liquid saturation at the turning point)
   * @param slDel the value of the liquid saturation at the turning point
   */
  Real landSat(Real slDel) const;
};
