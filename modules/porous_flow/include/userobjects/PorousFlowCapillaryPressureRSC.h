//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

  virtual Real capillaryPressureCurve(Real saturation) const override;
  virtual Real dCapillaryPressureCurve(Real saturation) const override;
  virtual Real d2CapillaryPressureCurve(Real saturation) const override;

  virtual Real effectiveSaturation(Real pc) const override;
  virtual Real dEffectiveSaturation(Real pc) const override;
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
