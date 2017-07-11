/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCAPILLARYPRESSURERSC_H
#define POROUSFLOWCAPILLARYPRESSURERSC_H

#include "PorousFlowCapillaryPressure.h"

class PorousFlowCapillaryPressureRSC;

template <>
InputParameters validParams<PorousFlowCapillaryPressureRSC>();

/**
 * Rogers-Stallybrass-Clements form of capillary pressure
 */
class PorousFlowCapillaryPressureRSC : public PorousFlowCapillaryPressure
{
public:
  PorousFlowCapillaryPressureRSC(const InputParameters & parameters);

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
  /// Oil viscosity (which must be twice the water viscocity in this formulation)
  const Real _oil_viscosity;
  /// Scale ratio: porosity/permeability/beta^2, where beta is chosen by the user
  const Real _scale_ratio;
  /// Shift.  seff_water = 1/Sqrt(1 + Exp((Pc - shift)/scale)), where scale = 0.25 * scale_ratio * oil_viscosity
  const Real _shift;
  /// Scale = 0.25 * scale_ratio * oil_viscosity
  const Real _scale;
};

#endif // POROUSFLOWCAPILLARYPRESSURERSC_H
