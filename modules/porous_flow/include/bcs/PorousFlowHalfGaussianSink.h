/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWHALFGAUSSIANSINK_H
#define POROUSFLOWHALFGAUSSIANSINK_H

#include "PorousFlowSinkPTDefiner.h"

// Forward Declarations
class PorousFlowHalfGaussianSink;

template <>
InputParameters validParams<PorousFlowHalfGaussianSink>();

/**
 * Applies a flux sink to a boundary.  The base flux
 * defined by PorousFlowSink is multiplied by a
 * _maximum*exp(-(0.5*(p - c)/_sd)^2)*_m_func for p<c
 * _maximum*_m_func for p>=c
 * Here p = porepressure for fluid fluxes, or p = temperature for heat fluxes.
 * This is typically used for modelling evapotranspiration
 * from the top of a groundwater model
 */
class PorousFlowHalfGaussianSink : public PorousFlowSinkPTDefiner
{
public:
  PorousFlowHalfGaussianSink(const InputParameters & parameters);

protected:
  /// maximum of the Gaussian sink
  const Real _maximum;

  /// standard deviation of the Gaussian sink
  const Real _sd;

  /// center of the Gaussian sink
  const Real _center;

  virtual Real multiplier() const override;

  virtual Real dmultiplier_dvar(unsigned int pvar) const override;
};

#endif // POROUSFLOWHALFGAUSSIANSINK_H
