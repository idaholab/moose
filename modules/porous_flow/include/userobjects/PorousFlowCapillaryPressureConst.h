/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
