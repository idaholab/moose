//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

  virtual Real capillaryPressureCurve(Real saturation) const override;
  virtual Real dCapillaryPressureCurve(Real saturation) const override;
  virtual Real d2CapillaryPressureCurve(Real saturation) const override;

  virtual Real effectiveSaturation(Real pc) const override;
  virtual Real dEffectiveSaturation(Real pc) const override;
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
