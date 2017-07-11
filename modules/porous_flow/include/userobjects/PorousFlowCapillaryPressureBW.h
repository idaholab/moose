/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCAPILLARYPRESSUREBW_H
#define POROUSFLOWCAPILLARYPRESSUREBW_H

#include "PorousFlowCapillaryPressure.h"

class PorousFlowCapillaryPressureBW;

template <>
InputParameters validParams<PorousFlowCapillaryPressureBW>();

/**
 * Capillary pressure of Broadbridge and White.
 */
class PorousFlowCapillaryPressureBW : public PorousFlowCapillaryPressure
{
public:
  PorousFlowCapillaryPressureBW(const InputParameters & parameters);

  /**
   * Capillary pressure is calculated as a function of effective saturation
   * @param seff effecitve saturation
   * @return capillary pressure (Pa)
   */
  virtual Real capillaryPressure(Real seff) const override;

  /**
   * Derivative of capillary pressure wrt effective saturation
   * @param seff effecitve saturation
   * @return derivative of capillary pressure with respect to liquid saturation
   */
  virtual Real dCapillaryPressure(Real seff) const override;

  /**
   * Second derivative of capillary pressure wrt effective saturation
   * @param seff effecitve saturation
   * @return second derivative of capillary pressure with respect to liquid saturation
   */
  virtual Real d2CapillaryPressure(Real seff) const override;

  /**
   * Effective saturation as a function of capillary pressure
   * @param pc capillary pressure (Pa)
   * @return effective saturation
   */
  virtual Real effectiveSaturation(Real pc) const override;

  /**
   * Derivative of effective saturation wrt capillary pressure
   * @param pc capillary pressure (Pa)
   * @return derivative of effective saturation wrt capillary pressure
   */
  virtual Real dEffectiveSaturation(Real pc) const override;

  /**
   * Second derivative of effective saturation wrt capillary pressure
   * @param pc capillary pressure
   * @return second derivative of effective saturation wrt capillary pressure
   */
  virtual Real d2EffectiveSaturation(Real pc) const override;

protected:
  /// BW's Sn parameter (initial saturation)
  const Real _sn;
  /// BW's Ss parameter
  const Real _ss;
  /// BW's C parameter (>1)
  const Real _c;
  /// BWs lambda_s parameter multiplied by fluid density * gravity (>0)
  const Real _las;
};

#endif // POROUSFLOWCAPILLARYPRESSUREBW_H
