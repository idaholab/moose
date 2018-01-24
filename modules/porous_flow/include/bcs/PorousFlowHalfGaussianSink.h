//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
