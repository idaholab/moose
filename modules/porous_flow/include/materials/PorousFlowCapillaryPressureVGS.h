/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCAPILLARYPRESSUREVGS_H
#define POROUSFLOWCAPILLARYPRESSUREVGS_H

#include "PorousFlowCapillaryPressureBase.h"

//Forward Declarations
class PorousFlowCapillaryPressureVGS;

template<>
InputParameters validParams<PorousFlowCapillaryPressureVGS>();

/**
 * van Genuchten capillary pressure for multiphase flow in porous media.
 *
 * Based on van Genuchten, M. Th., A closed for equation for
 * predicting the hydraulic conductivity of unsaturated soils,
 * Soil Sci. Soc., 44, 892-898 (1980).
 */
class PorousFlowCapillaryPressureVGS : public PorousFlowCapillaryPressureBase
{
public:
  PorousFlowCapillaryPressureVGS(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Capillary pressure as a function of phase saturation
  Real capillaryPressure(Real saturation) const;

  /// Derivative of capillary pressure wrt phase saturation
  Real dCapillaryPressure(Real saturation) const;

  /// Second derivative of capillary pressure wrt phase saturation
  Real d2CapillaryPressure(Real saturation) const;

  /// Effective saturation
  Real effectiveSaturation(Real saturation) const;

  /// Liquid residual saturation
  const Real _sat_lr;

  /// Liquid phase fully saturated saturation
  const Real _sat_ls;

  /// van Genuchten exponent m
  const Real _m;

  /// Maximum capillary pressure
  const Real _pc_max;

  /// van Genuchten capillary pressure coefficient
  const Real _p0;
};

#endif //POROUSFLOWCAPILLARYPRESSUREVGS_H
