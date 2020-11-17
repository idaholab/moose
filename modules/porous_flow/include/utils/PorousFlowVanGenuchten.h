//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

/**
 * van Genuchten effective saturation, capillary pressure and relative
 * permeability functions.
 * Note: effective saturation is provided as a function of porepressure, not
 * capillary pressure.
 * Note: capillary pressure and relative permeability are functions of effective
 * saturation. The derivatives are therefore given wrt effective saturation. These
 * derivatives must be multiplied by the derivative of effective saturation wrt
 * the true saturation in objects using these relations.
 *
 * Based on van Genuchten, M. Th., A closed for equation for
 * predicting the hydraulic conductivity of unsaturated soils,
 * Soil Sci. Soc., 44, 892-898 (1980).
 */

namespace PorousFlowVanGenuchten
{
/**
 * Effective saturation as a function of porepressure.
 * Note: seff = 1 for p >= 0
 * @param p porepressure
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @return effective saturation
 */
Real effectiveSaturation(Real p, Real alpha, Real m);

/**
 * Derivative of effective saturation wrt porepressure
 * @param p porepressure
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @return derivative of effective saturation wrt porepressure
 */
Real dEffectiveSaturation(Real p, Real alpha, Real m);

/**
 * Second derivative of effective saturation wrt porepressure
 * @param p porepressure
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @return second derivative of effective saturation wrt porepressure
 */
Real d2EffectiveSaturation(Real p, Real alpha, Real m);

/**
 * Capillary pressure as a function of effective saturation
 *
 * @param seff effective saturation
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @param pc_max maximum capillary pressure (Pa)
 * @return capillary pressure (Pa)
 */
Real capillaryPressure(Real seff, Real alpha, Real m, Real pc_max);

/**
 * Derivative of capillary pressure wrt effective saturation
 *
 * @param seff effective saturation
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @param pc_max maximum capillary pressure (Pa)
 * @return derivative of capillary pressure wrt effective saturation
 */
Real dCapillaryPressure(Real seff, Real alpha, Real m, Real pc_max);

/**
 * Second derivative of capillary pressure wrt effective saturation
 *
 * @param seff effective saturation
 * @param alpha van Genuchten parameter
 * @param m van Genuchten exponent
 * @param pc_max maximum capillary pressure (Pa)
 * @return second derivative of capillary pressure wrt effective saturation
 */
Real d2CapillaryPressure(Real seff, Real alpha, Real m, Real pc_max);

/**
 * Relative permeability as a function of effective saturation
 * @param seff effective saturation
 * @param m van Genuchten exponent
 * @return relative permeability
 */
Real relativePermeability(Real seff, Real m);

/**
 * Derivative of relative permeability with respect to effective saturation
 * @param seff effective saturation
 * @param m van Genuchten exponent
 * @return derivative of relative permeability wrt effective saturation
 */
Real dRelativePermeability(Real seff, Real m);

/**
 * Second derivative of relative permeability with respect to effective saturation
 * @param seff effective saturation
 * @param m van Genuchten exponent
 * @return second derivative of relative permeability wrt effective saturation
 */
Real d2RelativePermeability(Real seff, Real m);

/**
 * Relative permeability for a non-wetting phase as a function of effective saturation
 * @param seff effective saturation
 * @param m van Genuchten exponent
 * @return relative permeability
 */
Real relativePermeabilityNW(Real seff, Real m);

/**
 * Derivative of relative permeability for a non-wetting phase with respect to effective saturation
 * @param seff effective saturation
 * @param m van Genuchten exponent
 * @return derivative of relative permeability wrt effective saturation
 */
Real dRelativePermeabilityNW(Real seff, Real m);

/**
 * Second derivative of relative permeability for a non-wetting phase with respect to effective
 * saturation
 * @param seff effective saturation
 * @param m van Genuchten exponent
 * @return second derivative of relative permeability wrt effective saturation
 */
Real d2RelativePermeabilityNW(Real seff, Real m);

/**
 * Parameters associated with the extension of the hysteretic capillary pressure function to low
 * saturation values
 * @ ExtensionStrategy the type of extension used
 * @ S liquid saturation at the point of extension
 * @ Pc capillary pressure at the point of extension
 * @ dPc d(Pc)/dS at the point of extension
 */
struct LowCapillaryPressureExtension
{
  enum ExtensionStrategy
  {
    NONE,
    QUADRATIC,
    EXPONENTIAL
  };
  ExtensionStrategy strategy;
  Real S;
  Real Pc;
  Real dPc;

  LowCapillaryPressureExtension()
    : strategy(LowCapillaryPressureExtension::NONE),
      S(0.0),
      Pc(std::numeric_limits<Real>::max()),
      dPc(std::numeric_limits<Real>::lowest()){};

  LowCapillaryPressureExtension(const ExtensionStrategy & strategy, Real S, Real Pc, Real dPc)
    : strategy(strategy), S(S), Pc(Pc), dPc(dPc){};
};

/**
 * Parameters associated with the extension of the hysteretic wetting capillary pressure function to
 * high saturation values
 * @ ExtensionStrategy the type of extension used
 * @ S liquid saturation at the point of extension
 * @ Pc capillary pressure at the point of extension
 * @ dPc d(Pc)/dS at the point of extension
 */
struct HighCapillaryPressureExtension
{
  enum ExtensionStrategy
  {
    NONE,
    POWER
  };
  ExtensionStrategy strategy;
  Real S;
  Real Pc;
  Real dPc;

  HighCapillaryPressureExtension()
    : strategy(HighCapillaryPressureExtension::NONE),
      S(1.0),
      Pc(0.0),
      dPc(std::numeric_limits<Real>::lowest()){};

  HighCapillaryPressureExtension(const ExtensionStrategy & strategy, Real S, Real Pc, Real dPc)
    : strategy(strategy), S(S), Pc(Pc), dPc(dPc){};
};

/**
 * Hysteretic capillary pressure function (Eqn(1) of Doughty2007) with extensions (page5 and Fig1 of
 * Doughty2008).
 * NOTE: this function is undefined for sl < 0 and sl > 1, so you MUST ensure 0 <= sl <= 1
 * NOTE: this returns a non-negative quantity.
 * @param sl liquid saturation. 0 <= sl <= 1
 * @param slmin value of liquid sat where the van Genuchten expression -> infinity.  0 <= slmin < 1
 * @param sgrdel value of gas saturation where van Genuchten expression -> 0.  slmin < 1 - Sgrdel <=
 * 1
 * @param alpha van Genuchten alpha parameter, with dimensions 1/Pa.  alpha > 0
 * @param n van Genuchten n parameter.  n > 1
 * @param low_ext strategy and parameters to use for the extension in the small-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 * @param high_ext strategy and parameters to use for the extension in the high-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 */
Real capillaryPressureHys(
    Real sl,
    Real slmin,
    Real sgrdel,
    Real alpha,
    Real n,
    const LowCapillaryPressureExtension & low_ext = LowCapillaryPressureExtension(),
    const HighCapillaryPressureExtension & high_ext = HighCapillaryPressureExtension());

/**
 * Derivative of capillaryPressureHys with respect to sl.
 * NOTE: this function is undefined for sl < 0 and sl > 1, so you MUST ensure 0 <= sl <= 1
 * NOTE: this returns a negative quantity.
 * @param sl liquid saturation. 0 <= sl <= 1
 * @param slmin value of liquid sat where the van Genuchten expression -> infinity.  0 <= slmin < 1
 * @param sgrdel value of gas saturation where van Genuchten expression -> 0.  slmin < 1 - Sgrdel <=
 * 1
 * @param alpha van Genuchten alpha parameter, with dimensions 1/Pa.  alpha > 0
 * @param n van Genuchten n parameter.  n > 1
 * @param low_ext strategy and parameters to use for the extension in the small-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 * @param high_ext strategy and parameters to use for the extension in the high-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 */
Real dcapillaryPressureHys(
    Real sl,
    Real slmin,
    Real sgrdel,
    Real alpha,
    Real n,
    const LowCapillaryPressureExtension & low_ext = LowCapillaryPressureExtension(),
    const HighCapillaryPressureExtension & high_ext = HighCapillaryPressureExtension());

/**
 * Second derivative of capillaryPressureHys with respect to sl.
 * NOTE: this function is undefined for sl < 0 and sl > 1, so you MUST ensure 0 <= sl <= 1
 * @param sl liquid saturation. 0 <= sl <= 1
 * @param slmin value of liquid sat where the van Genuchten expression -> infinity.  0 <= slmin < 1
 * @param sgrdel value of gas saturation where van Genuchten expression -> 0.  slmin < 1 - Sgrdel <=
 * 1
 * @param alpha van Genuchten alpha parameter, with dimensions 1/Pa.  alpha > 0
 * @param n van Genuchten n parameter.  n > 1
 * @param low_ext strategy and parameters to use for the extension in the small-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 * @param high_ext strategy and parameters to use for the extension in the high-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 */
Real d2capillaryPressureHys(
    Real sl,
    Real slmin,
    Real sgrdel,
    Real alpha,
    Real n,
    const LowCapillaryPressureExtension & low_ext = LowCapillaryPressureExtension(),
    const HighCapillaryPressureExtension & high_ext = HighCapillaryPressureExtension());

/**
 * Hysteretic saturation function (Eqn(1) of Doughty2007) with extensions (page5 and Fig1 of
 * Doughty2008), which is the inverse of capillaryPressureHys
 * @param pc capillary pressure.  0 <= pc
 * @param slmin value of liquid sat where the van Genuchten expression -> infinity.  0 <= slmin < 1
 * @param sgrdel value of gas saturation where van Genuchten expression -> 0.  slmin < 1 - Sgrdel <=
 * 1
 * @param alpha van Genuchten alpha parameter, with dimensions 1/Pa.  alpha > 0
 * @param n van Genuchten n parameter.  n > 1
 * @param low_ext strategy and parameters to use for the extension in the small-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 * @param high_ext strategy and parameters to use for the extension in the high-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 */
Real
saturationHys(Real pc,
              Real slmin,
              Real sgrdel,
              Real alpha,
              Real n,
              const LowCapillaryPressureExtension & low_ext = LowCapillaryPressureExtension(),
              const HighCapillaryPressureExtension & high_ext = HighCapillaryPressureExtension());

/**
 * Derivative of Hysteretic saturation function with respect to pc
 * @param pc capillary pressure.  0 <= pc
 * @param slmin value of liquid sat where the van Genuchten expression -> infinity.  0 <= slmin < 1
 * @param sgrdel value of gas saturation where van Genuchten expression -> 0.  slmin < 1 - Sgrdel <=
 * 1
 * @param alpha van Genuchten alpha parameter, with dimensions 1/Pa.  alpha > 0
 * @param n van Genuchten n parameter.  n > 1
 * @param low_ext strategy and parameters to use for the extension in the small-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 * @param high_ext strategy and parameters to use for the extension in the high-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 */
Real
dsaturationHys(Real pc,
               Real slmin,
               Real sgrdel,
               Real alpha,
               Real n,
               const LowCapillaryPressureExtension & low_ext = LowCapillaryPressureExtension(),
               const HighCapillaryPressureExtension & high_ext = HighCapillaryPressureExtension());

/**
 * Second derivative of Hysteretic saturation function with respect to pc
 * @param pc capillary pressure.  0 <= pc
 * @param slmin value of liquid sat where the van Genuchten expression -> infinity.  0 <= slmin < 1
 * @param sgrdel value of gas saturation where van Genuchten expression -> 0.  slmin < 1 - Sgrdel <=
 * 1
 * @param alpha van Genuchten alpha parameter, with dimensions 1/Pa.  alpha > 0
 * @param n van Genuchten n parameter.  n > 1
 * @param low_ext strategy and parameters to use for the extension in the small-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 * @param high_ext strategy and parameters to use for the extension in the high-saturation region
 * (defaults to no extension: this default is not recommended for simulations of real phenomena)
 */
Real
d2saturationHys(Real pc,
                Real slmin,
                Real sgrdel,
                Real alpha,
                Real n,
                const LowCapillaryPressureExtension & low_ext = LowCapillaryPressureExtension(),
                const HighCapillaryPressureExtension & high_ext = HighCapillaryPressureExtension());
}
