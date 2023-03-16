//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiscreteElementUserObject.h"

/**
 * Base class for capillary pressure for multiphase flow in porous media. To implement
 * an effective saturation formulation, override effectiveSaturation() and derivatives.
 * To implement a capillary pressure curve that will include the logarithmic extension,
 * override capillaryPressureCurve() and derivatives.
 *
 * Note: Capillary pressure is calculated as a function of true saturation, not
 * effective saturation (saturation minus residual). Derivatives are returned wrt
 * true saturation, so no scaling of them is required in objects using these methods.
 *
 * Includes an optional logarithmic extension in the low saturation region where
 * capillary pressure can go to infinity as saturation tends to 0. Calculation of
 * logarithmic extension from Webb, A simple extension of two-phase characteristic
 * curves to include the dry region, Water Resources Research 36, 1425-1430 (2000)
 * The logarithmic extension is only computed for qp=0.  If your overriding class
 * has no qp dependence (capillary pressure is a function only of saturation, for
 * example) then no changes are required.  Otherwise, if you use log_extension,
 * you will have to modify this base class to correctly calculate the log-extension
 * parameters at each quadpoint.
 */
class PorousFlowCapillaryPressure : public DiscreteElementUserObject
{
public:
  static InputParameters validParams();

  PorousFlowCapillaryPressure(const InputParameters & parameters);

  virtual void initialize() final{};
  virtual void initialSetup() override;

  /**
   * Capillary pressure is calculated as a function of true saturation. Note that this
   * method includes the ability to use a logarithmic extension at low saturation.
   * @param saturation true saturation
   * @param qp quadpoint to use (when capillary-pressure depends on coupled variables, not just
   * saturation)
   * @return capillary pressure (Pa)
   */
  virtual Real capillaryPressure(Real saturation, unsigned qp = 0) const;
  virtual DualReal capillaryPressure(const DualReal & saturation, unsigned qp = 0) const;

  /**
   * Derivative of capillary pressure wrt true saturation
   * @param saturation true saturation
   * @param qp quadpoint to use (when capillary-pressure depends on coupled variables, not just
   * saturation)
   * @return derivative of capillary pressure with respect to true saturation
   */
  virtual Real dCapillaryPressure(Real saturation, unsigned qp = 0) const;

  /**
   * Second derivative of capillary pressure wrt true saturation
   * @param saturation true saturation
   * @param qp quadpoint to use (when capillary-pressure depends on coupled variables, not just
   * saturation)
   * @return second derivative of capillary pressure with respect to true saturation
   */
  virtual Real d2CapillaryPressure(Real saturation, unsigned qp = 0) const;

  /**
   * Effective saturation as a function of capillary pressure
   * @param pc capillary pressure (Pa)
   * @param qp quadpoint to use (when effective saturation depends on coupled variables, not just
   * pc)
   * @return effective saturation
   */
  virtual Real effectiveSaturation(Real pc, unsigned qp = 0) const = 0;

  /**
   * Derivative of effective saturation wrt capillary pressure
   * @param pc capillary pressure (Pa)
   * @param qp quadpoint to use (when effective saturation depends on coupled variables, not just
   * pc)
   * @return derivative of effective saturation wrt capillary pressure
   */
  virtual Real dEffectiveSaturation(Real pc, unsigned qp = 0) const = 0;

  /**
   * Second derivative of effective saturation wrt capillary pressure
   * @param pc capillary pressure
   * @param qp quadpoint to use (when effective saturation depends on coupled variables, not just
   * pc)
   * @return second derivative of effective saturation wrt capillary pressure
   */
  virtual Real d2EffectiveSaturation(Real pc, unsigned qp = 0) const = 0;

  /**
   * Saturation as a function of capillary pressure
   * @param pc capillary pressure (Pa)
   * @param qp quadpoint to use (when saturation depends on coupled variables, not just
   * pc)
   * @return saturation
   */
  Real saturation(Real pc, unsigned qp = 0) const;
  DualReal saturation(const DualReal & pc, unsigned qp = 0) const;

  /**
   * Derivative of saturation wrt capillary pressure
   * @param pc capillary pressure (Pa)
   * @param qp quadpoint to use (when saturation depends on coupled variables, not just
   * pc)
   * @return derivative of saturation wrt capillary pressure
   */
  Real dSaturation(Real pc, unsigned qp = 0) const;

  /**
   * Second derivative of saturation wrt capillary pressure
   * @param pc capillary pressure
   * @param qp quadpoint to use (when saturation depends on coupled variables, not just
   * pc)
   * @return second derivative of saturation wrt capillary pressure
   */
  Real d2Saturation(Real pc, unsigned qp = 0) const;

protected:
  /**
   * Effective saturation of liquid phase given liquid saturation and residual
   * liquid saturation.
   * Note: not to be mistaken with effectiveSaturation(pc) which is a function
   * of capillary pressure.
   * @param saturation true saturation
   * @return effective saturation
   */
  Real effectiveSaturationFromSaturation(Real saturation) const;

  /**
   * Calculates the saturation where the logarithmic extension to capillary
   * pressure meets the raw curve using Newton's method.
   * This implementation assumes capillary-pressure is a function of saturation
   * only, and not any other quad-point dependent quantities
   *
   * @return saturation where logarithmic extension begins
   */
  Real extensionSaturation() const;

  /**
   * Calculates the saturation where the logarithmic extension to capillary
   * pressure at low saturation.  This is computed for qp=0 only.
   * This implementation assumes capillary-pressure is a function of saturation
   * only, and not any other quad-point dependent quantities
   *
   * @param s effective saturation
   * @return capillary pressure function in the logarithmic extension
   */
  Real interceptFunction(Real s) const;

  /**
   * Calculates the saturation where the logarithmic extension to capillary
   * pressure at low saturation.
   * This implementation assumes capillary-pressure is a function of saturation
   * only, and not any other quad-point dependent quantities
   *
   * @param s effective saturation
   * @return derivative of logarithmic extension function
   */
  Real interceptFunctionDeriv(Real s) const;

  /**
   * The capillary pressure in the logarithmic extension
   * This implementation assumes capillary-pressure is a function of saturation
   * only, and not any other quad-point dependent quantities
   *
   * @param s liquid saturation
   * @return capillary pressure in logarithmic extension
   */
  Real capillaryPressureLogExt(Real s) const;

  /**
   * The derivative of capillary pressure in the logarithmic extension
   * This implementation assumes capillary-pressure is a function of saturation
   * only, and not any other quad-point dependent quantities
   *
   * @param s liquid saturation
   * @return derivative of capillary pressure in logarithmic extension
   */
  Real dCapillaryPressureLogExt(Real s) const;

  /**
   * The second derivative of capillary pressure in the logarithmic extension
   * This implementation assumes capillary-pressure is a function of saturation
   * only, and not any other quad-point dependent quantities
   *
   * @param s liquid saturation
   * @return second derivative of capillary pressure in logarithmic extension
   */
  Real d2CapillaryPressureLogExt(Real s) const;

  /**
   * Raw capillary pressure curve (does not include logarithmic extension)
   * @param saturation true saturation
   * @return capillary pressure (Pa)
   */
  virtual Real capillaryPressureCurve(Real saturation, unsigned qp = 0) const = 0;

  /**
   * Derivative of raw capillary pressure wrt true saturation
   * @param saturation true saturation
   * @return derivative of capillary pressure with respect to true saturation
   */
  virtual Real dCapillaryPressureCurve(Real saturation, unsigned qp = 0) const = 0;

  /**
   * Second derivative of raw capillary pressure wrt true saturation
   * @param saturation true saturation
   * @return second derivative of capillary pressure with respect to true saturation
   */
  virtual Real d2CapillaryPressureCurve(Real saturation, unsigned qp = 0) const = 0;

  /// Liquid residual saturation
  const Real _sat_lr;
  /// Derivative of effective saturation with respect to saturation
  const Real _dseff_ds;
  /// Flag to use a logarithmic extension for low saturation
  bool _log_ext;
  /// Maximum capillary pressure (Pa). Note: must be <= 0
  const Real _pc_max;

  /**
   * Saturation where the logarithmic extension meets the raw curve
   * This is computed only for qp=0.
   */
  Real _sat_ext;
  /**
   * Capillary pressure where the extension meets the raw curve.
   * This is computed only for qp=0.
   */
  Real _pc_ext;
  /**
   * Gradient of the logarithmic extension
   * This is computed only for qp=0.
   */
  Real _slope_ext;
  /// log(10)
  const Real _log10;
};
