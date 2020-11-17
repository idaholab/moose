//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowVariableBase.h"
#include "PorousFlowConstants.h"
#include "PorousFlowVanGenuchten.h"

/**
 * Base material designed to calculate and store quantities relevant for hysteretic capillary
 * pressure calculations
 */
class PorousFlowHystereticCapillaryPressure : public PorousFlowVariableBase
{
public:
  static InputParameters validParams();

  PorousFlowHystereticCapillaryPressure(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * @return the value of gas saturation (called S_gr^Delta in the markdown documentation) using the
   * Land expression.  (This is a function of the liquid saturation at the turning point)
   * @param slDel the value of the liquid saturation at the turning point
   */
  Real landSat(Real slDel) const;

  /**
   * Compute all relevant quantities at the given turning point
   * @param tp_num The turning point number.  Eg, tp_num=0 upon transition from zeroth-order drying
   * to first-order wetting
   * @param tp_sat Liquid saturation at the turning point
   * @param tp_pc Capillary pressure at the turning point
   */
  void computeTurningPointInfo(unsigned tp_num, Real tp_sat, Real tp_pc);

  /**
   * @return the value of capillary pressure, given the liquid saturation.  This uses
   * _hys_order[_qp]
   * @param sat liquid saturation
   */
  Real capillaryPressureQp(Real sat) const;

  /**
   * @return d(capillary pressure)/d(sat).  This uses _hys_order[_qp]
   * @param sat liquid saturation
   */
  Real dcapillaryPressureQp(Real sat) const;

  /**
   * @return d^2(capillary pressure)/d(sat)^2.  This uses _hys_order[_qp]
   * @param sat liquid saturation
   */
  Real d2capillaryPressureQp(Real sat) const;

  /**
   * @return the value of liquid saturation, given the capillary pressure.  This uses
   * _hys_order[_qp]
   * @param pc capillary pressure
   */
  Real liquidSaturationQp(Real pc) const;

  /**
   * @return d(liquid saturation)/d(pc).  This uses _hys_order[_qp]
   * @param pc capillary pressure
   */
  Real dliquidSaturationQp(Real pc) const;

  /**
   * @return d^2(liquid saturation)/d(pc)^2.  This uses _hys_order[_qp]
   * @param pc capillary pressure
   */
  Real d2liquidSaturationQp(Real pc) const;

  /// van Genuchten alpha parameter for the primary drying curve
  const Real _alpha_d;
  /// van Genuchten alpha parameter for the primary wetting curve
  const Real _alpha_w;
  /// van Genuchten n parameter for the primary drying curve
  const Real _n_d;
  /// van Genuchten n parameter for the primary wetting curve
  const Real _n_w;
  /// Minimum liquid saturation for which the van Genuchten expression is valid (Pc(_s_l_min) = infinity)
  const Real _s_l_min;
  /// Liquid saturation below which the liquid relative permeability is zero
  const Real _s_lr;
  /// Residual gas saturation: 1 - _s_gr_max is the maximum saturation for which the van Genuchten expression is valid for the wetting curve
  const Real _s_gr_max;
  /// Maximum capillary pressure: for Pc above this value, a "lower" extension will be used
  const Real _pc_max;
  /// The high-saturation extension to the wetting will commence at _high_ratio * (1 - _s_gr_del)
  const Real _high_ratio;
  /// Type of low-saturation extension
  const PorousFlowVanGenuchten::LowCapillaryPressureExtension::ExtensionStrategy _low_ext_type;
  /// Saturation on the primary drying curve where low-saturation extension commences
  const Real _s_low_d;
  /// d(Pc)/dS on the primary drying curve at S = _s_low_d
  const Real _dpc_low_d;
  /// Parameters involved in the low-saturation extension of the primary drying curve
  const PorousFlowVanGenuchten::LowCapillaryPressureExtension _low_ext_d;
  /// Saturation on the primary wetting curve where low-saturation extension commences
  const Real _s_low_w;
  /// d(Pc)/dS on the primary wetting curve at S = _s_low_w
  const Real _dpc_low_w;
  /// Parameters involved in the low-saturation extension of the primary wetting curve
  const PorousFlowVanGenuchten::LowCapillaryPressureExtension _low_ext_w;
  /// Type of high-saturation extension of the wetting curves
  const PorousFlowVanGenuchten::HighCapillaryPressureExtension::ExtensionStrategy _high_ext_type;
  /// Saturation at the point of high-saturation extension
  const Real _s_high;
  /// Pc at the point of high-saturation extension
  const Real _pc_high;
  /// d(Pc)/d(S) at the point of high-saturation extension
  const Real _dpc_high;
  /// Parameters involved in the high-saturation extension of the primary wetting curve
  const PorousFlowVanGenuchten::HighCapillaryPressureExtension _high_ext;

  /// Hysteresis order, as computed by PorousFlowHysteresisOrder
  const MaterialProperty<unsigned> & _hys_order;

  /// Old value of hysteresis order, as computed by PorousFlowHysteresisOrder
  const MaterialProperty<unsigned> & _hys_order_old;

  /// Saturation values at the turning points, as computed by PorousFlowHysteresisOrder
  const MaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>> &
      _hys_sat_tps;

  /// Older value of capillary pressure
  const MaterialProperty<Real> & _pc_older;

  /// Nodal or quadpoint values of Pc at the turning points.
  MaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>> & _pc_tps;

  /// Computed nodal or quadpoint values of saturation on the drying curve at _pc_tps
  MaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>> & _s_d_tps;

  /// Computed nodal or quadpoint values of S_gr_Del, ie, the Land expression, at the turning points
  MaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>> & _s_gr_tps;

  /// Nodal or quadpoint values of the low extension of the wetting curve defined by _s_gr_tps
  MaterialProperty<std::array<PorousFlowVanGenuchten::LowCapillaryPressureExtension,
                              PorousFlowConstants::MAX_HYSTERESIS_ORDER>> & _w_low_ext_tps;

  /// Nodal or quadpoint values of the high extension of the wetting curve defined by _s_gr_tps
  MaterialProperty<std::array<PorousFlowVanGenuchten::HighCapillaryPressureExtension,
                              PorousFlowConstants::MAX_HYSTERESIS_ORDER>> & _w_high_ext_tps;

  /// Computed nodal or quadpoint values of liquid saturation on the wetting curve defined by _s_gr_del, at pc = _pc_d_tps
  MaterialProperty<std::array<Real, PorousFlowConstants::MAX_HYSTERESIS_ORDER>> & _s_w_tps;

private:
  /**
   * @return capillary pressure on the first-order wetting curve
   * @param sat liquid saturation
   */
  Real firstOrderWettingPc(Real sat) const;

  /**
   * @return d(capillary pressure on the first-order wetting curve)/d(sat)
   * @param sat liquid saturation
   */
  Real dfirstOrderWettingPc(Real sat) const;

  /**
   * @return d^2(capillary pressure on the first-order wetting curve)/d(sat)^2
   * @param sat liquid saturation
   */
  Real d2firstOrderWettingPc(Real sat) const;

  /**
   * @return capillary pressure on the second-order drying curve
   * @param sat liquid saturation
   */
  Real secondOrderDryingPc(Real sat) const;

  /**
   * @return d(capillary pressure on the second-order drying curve)/d(sat)
   * @param sat liquid saturation
   */
  Real dsecondOrderDryingPc(Real sat) const;

  /**
   * @return d^2(capillary pressure on the second-order drying curve)/d(sat)^2
   * @param sat liquid saturation
   */
  Real d2secondOrderDryingPc(Real sat) const;

  /**
   * @return saturation on the first-order wetting curve
   * @param pc capillary pressure
   */
  Real firstOrderWettingSat(Real pc) const;

  /**
   * @return d(saturation on the first-order wetting curve)/d(pc)
   * @param pc capillary pressure
   */
  Real dfirstOrderWettingSat(Real pc) const;

  /**
   * @return d^2(saturation on the first-order wetting curve)/d(pc)^2
   * @param pc capillary pressure
   */
  Real d2firstOrderWettingSat(Real pc) const;

  /**
   * @return saturation on the second-order drying curve
   * @param pc capillary pressure
   */
  Real secondOrderDryingSat(Real pc) const;

  /**
   * @return d(saturation on the second-order drying curve)/d(pc)
   * @param pc capillary pressure
   */
  Real dsecondOrderDryingSat(Real pc) const;

  /**
   * @return d^2(saturation on the second-order drying curve)/d(pc)^2
   * @param pc capillary pressure
   */
  Real d2secondOrderDryingSat(Real pc) const;
};
