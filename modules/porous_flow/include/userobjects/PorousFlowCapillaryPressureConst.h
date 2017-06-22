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

  /**
   * Capillary pressure is calculated as a function of saturation
   * @param saturation true saturation
   * @return capillary pressure (Pa)
   */
  virtual Real capillaryPressure(Real saturation) const override;

  /**
   * Derivative of capillary pressure wrt saturation
   * @param saturation true saturation
   * @return derivative of capillary pressure with respect to liquid saturation
   */
  virtual Real dCapillaryPressure(Real saturation) const override;

  /**
   * Second derivative of capillary pressure wrt saturation
   * @param saturation true saturation
   * @return second derivative of capillary pressure with respect to liquid saturation
   */
  virtual Real d2CapillaryPressure(Real saturation) const override;

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
  /// Constant capillary pressure (Pa)
  const Real _pc;
};

#endif // POROUSFLOWCAPILLARYPRESSURECONST_H
