/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCAPILLARYPRESSURE_H
#define POROUSFLOWCAPILLARYPRESSURE_H

#include "GeneralUserObject.h"

class PorousFlowCapillaryPressure;

template <>
InputParameters validParams<PorousFlowCapillaryPressure>();

/**
 * Base class for capillary pressure for multiphase flow in porous media.
 * Methods must be overriden in derived classes.
 * Note: Capillary pressure is calculated as a function of true saturation, not
 * effective saturation (saturation minus residual). Derivatives are returned wrt
 * true saturation, so no scaling of them is required in objects using these methods
 */
class PorousFlowCapillaryPressure : public GeneralUserObject
{
public:
  PorousFlowCapillaryPressure(const InputParameters & parameters);

  void initialize() final{};
  void execute() final{};
  void finalize() final{};

  /**
   * Capillary pressure is calculated as a function of true saturation
   * @param saturation true saturation
   * @return capillary pressure (Pa)
   */
  virtual Real capillaryPressure(Real saturation) const = 0;

  /**
   * Derivative of capillary pressure wrt true saturation
   * @param saturation true saturation
   * @return derivative of capillary pressure with respect to true saturation
   */
  virtual Real dCapillaryPressure(Real saturation) const = 0;

  /**
   * Second derivative of capillary pressure wrt true saturation
   * @param saturation true saturation
   * @return second derivative of capillary pressure with respect to true saturation
   */
  virtual Real d2CapillaryPressure(Real saturation) const = 0;

  /**
   * Effective saturation as a function of capillary pressure
   * @param pc capillary pressure (Pa)
   * @return effective saturation
   */
  virtual Real effectiveSaturation(Real pc) const = 0;

  /**
   * Derivative of effective saturation wrt capillary pressure
   * @param pc capillary pressure (Pa)
   * @return derivative of effective saturation wrt capillary pressure
   */
  virtual Real dEffectiveSaturation(Real pc) const = 0;

  /**
   * Second derivative of effective saturation wrt capillary pressure
   * @param pc capillary pressure
   * @return second derivative of effective saturation wrt capillary pressure
   */
  virtual Real d2EffectiveSaturation(Real pc) const = 0;

protected:
  /**
   * Effective saturation of liquid phase given liquid saturation and residual
   * liquid saturation.
   * Note: not to be mistaken with effectiveSaturation(pc) which is a function
   * of capillary pressure.
   * @param saturation true saturation
   * @return effective saturation
   */
  Real effectiveSaturationFromSaturation(Real saturation) const;

  /// Liquid residual saturation
  const Real _sat_lr;
  /// Derivative of effective saturation with respect to saturation
  const Real _dseff_ds;
};

#endif // POROUSFLOWCAPILLARYPRESSURE_H
