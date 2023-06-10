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
 * Material to calculate Brooks-Corey relative permeability of an arbitrary phase
 * given the effective saturation and exponent of that phase.
 *
 * From Brooks, R. H. and A. T. Corey (1966), Properties of porous media affecting
 * fluid flow, J. Irrig. Drain. Div., 6, 61
 */
template <bool is_ad>
class PorousFlowRelativePermeabilityBCTempl : public PorousFlowRelativePermeabilityBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowRelativePermeabilityBCTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> relativePermeability(GenericReal<is_ad> seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// Brooks-Corey exponent lambda
  const Real _lambda;
  /// Flag that is set to true if this is the non-wetting (gas) phase
  const bool _is_nonwetting;
};

typedef PorousFlowRelativePermeabilityBCTempl<false> PorousFlowRelativePermeabilityBC;
typedef PorousFlowRelativePermeabilityBCTempl<true> ADPorousFlowRelativePermeabilityBC;
