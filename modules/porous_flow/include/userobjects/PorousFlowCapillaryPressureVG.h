/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCAPILLARYPRESSUREVG_H
#define POROUSFLOWCAPILLARYPRESSUREVG_H

#include "PorousFlowCapillaryPressure.h"

class PorousFlowCapillaryPressureVG;

template <>
InputParameters validParams<PorousFlowCapillaryPressureVG>();

/**
 * van Genuchten form of capillary pressure.
 *
 * From van Genuchten, M. Th., A closed for equation for predicting the
 * hydraulic conductivity of unsaturated soils, Soil Sci. Soc., 44, 892-898 (1980)
 */
class PorousFlowCapillaryPressureVG : public PorousFlowCapillaryPressure
{
public:
  PorousFlowCapillaryPressureVG(const InputParameters & parameters);

  virtual Real capillaryPressureCurve(Real saturation) const override;
  virtual Real dCapillaryPressureCurve(Real saturation) const override;
  virtual Real d2CapillaryPressureCurve(Real saturation) const override;

  virtual Real effectiveSaturation(Real pc) const override;
  virtual Real dEffectiveSaturation(Real pc) const override;
  virtual Real d2EffectiveSaturation(Real pc) const override;

protected:
  /// van Genuchten exponent m
  const Real _m;
  /// van Genuchten capillary coefficient alpha
  const Real _alpha;
};

#endif // POROUSFLOWCAPILLARYPRESSUREVG_H
