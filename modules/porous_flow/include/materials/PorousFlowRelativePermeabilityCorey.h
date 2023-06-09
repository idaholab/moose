//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowRelativePermeabilityBase.h"

/**
 * Material to calculate Corey-type relative permeability of an arbitrary phase
 * given the effective saturation and Corey exponent of that phase.
 *
 * From  Corey, A. T., The interrelation between gas and oil relative
 * permeabilities, Prod. Month. 38-41 (1954)
 */
template <bool is_ad>
class PorousFlowRelativePermeabilityCoreyTempl
  : public PorousFlowRelativePermeabilityBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowRelativePermeabilityCoreyTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> relativePermeability(GenericReal<is_ad> seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// Corey exponent n for the specified phase
  const Real _n;
};

typedef PorousFlowRelativePermeabilityCoreyTempl<false> PorousFlowRelativePermeabilityCorey;
typedef PorousFlowRelativePermeabilityCoreyTempl<true> ADPorousFlowRelativePermeabilityCorey;
