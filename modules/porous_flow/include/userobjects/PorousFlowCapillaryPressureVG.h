//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  /// capillary pressure = f(Seff * s_scale) - pc_sscale, where f is the van Genuchten function.  For almost all simulations s_scale=1 will be appropriate
  const Real _s_scale;
  /// pc_sscale = f(s_scale), where f is the van Genuchten function
  const Real _pc_sscale;
};

#endif // POROUSFLOWCAPILLARYPRESSUREVG_H
