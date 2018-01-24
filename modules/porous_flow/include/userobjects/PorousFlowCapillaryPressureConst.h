//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWCAPILLARYPRESSURECONST_H
#define POROUSFLOWCAPILLARYPRESSURECONST_H

#include "PorousFlowCapillaryPressure.h"

class PorousFlowCapillaryPressureConst;

template <>
InputParameters validParams<PorousFlowCapillaryPressureConst>();

/**
 * Constant capillary pressure
 */
class PorousFlowCapillaryPressureConst : public PorousFlowCapillaryPressure
{
public:
  PorousFlowCapillaryPressureConst(const InputParameters & parameters);

  virtual Real capillaryPressureCurve(Real saturation) const override;
  virtual Real dCapillaryPressureCurve(Real saturation) const override;
  virtual Real d2CapillaryPressureCurve(Real saturation) const override;

  virtual Real effectiveSaturation(Real pc) const override;
  virtual Real dEffectiveSaturation(Real pc) const override;
  virtual Real d2EffectiveSaturation(Real pc) const override;

protected:
  /// Constant capillary pressure (Pa)
  const Real _pc;
};

#endif // POROUSFLOWCAPILLARYPRESSURECONST_H
