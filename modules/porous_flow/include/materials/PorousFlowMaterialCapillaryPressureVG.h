/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALCAPILLARYPRESSUREVG_H
#define POROUSFLOWMATERIALCAPILLARYPRESSUREVG_H

#include "PorousFlowMaterialCapillaryPressureBase.h"

//Forward Declarations
class PorousFlowMaterialCapillaryPressureVG;

template<>
InputParameters validParams<PorousFlowMaterialCapillaryPressureVG>();

/**
 * van Genuchten capillary pressure for multiphase flow in porous media.
 *
 * Based on van Genuchten, M. Th., A closed for equation for
 * predicting the hydraulic conductivity of unsaturated soils,
 * Soil Sci. Soc., 44, 892-898 (1980).
 */
class PorousFlowMaterialCapillaryPressureVG : public PorousFlowMaterialCapillaryPressureBase
{
public:
  PorousFlowMaterialCapillaryPressureVG(const InputParameters & parameters);

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
  Real _sat_lr;

  /// Liquid phase fully saturated saturation
  Real _sat_ls;

  /// van Genuchten exponent m
  Real _m;

  /// Maximum capillary pressure
  Real _pc_max;

  /// van Genuchten capillary pressure coefficient
  Real _p0;
};

#endif //POROUSFLOWMATERIALCAPILLARYPRESSUREVG_H
