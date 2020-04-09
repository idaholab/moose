//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowCapillaryPressure.h"

/**
 * Capillary pressure of Broadbridge and White.
 */
class PorousFlowCapillaryPressureBW : public PorousFlowCapillaryPressure
{
public:
  static InputParameters validParams();

  PorousFlowCapillaryPressureBW(const InputParameters & parameters);

  virtual Real capillaryPressureCurve(Real saturation, unsigned qp = 0) const override;
  virtual Real dCapillaryPressureCurve(Real saturation, unsigned qp = 0) const override;
  virtual Real d2CapillaryPressureCurve(Real saturation, unsigned qp = 0) const override;

  virtual Real effectiveSaturation(Real pc, unsigned qp = 0) const override;
  virtual Real dEffectiveSaturation(Real pc, unsigned qp = 0) const override;
  virtual Real d2EffectiveSaturation(Real pc, unsigned qp = 0) const override;

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
