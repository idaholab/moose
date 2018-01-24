//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWUNSATURATED_H
#define POROUSFLOWUNSATURATED_H

#include "PorousFlowSinglePhaseBase.h"

class PorousFlowUnsaturated;

template <>
InputParameters validParams<PorousFlowUnsaturated>();

/**
 * Action for simulation involving a single phase, partially or fully saturated fluid.
 * The fluid's saturation is found using the van-Genuchten expression, and the
 * relative permeability is found using the FLAC or Corey expression.
 */
class PorousFlowUnsaturated : public PorousFlowSinglePhaseBase
{
public:
  PorousFlowUnsaturated(const InputParameters & params);

  virtual void act() override;

protected:
  /// Add an Aux Variable to record saturation
  const bool _add_saturation_aux;

  /// Van Genuchten alpha parameter
  const Real _van_genuchten_alpha;

  /// Van Genuchten m parameter
  const Real _van_genuchten_m;

  /// Fluid relative permeability type (FLAC or Corey)
  const enum class RelpermTypeChoiceEnum { FLAC, COREY } _relperm_type;

  /// Relative permeability exponent
  const Real _relative_permeability_exponent;

  /// Residual saturation to use in the relative permeability expressions
  const Real _s_res;
};

#endif // POROUSFLOWUNSATURATED_H
