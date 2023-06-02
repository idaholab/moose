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
#include "PorousFlowFLACrelperm.h"

/**
 * Material to calculate relative permeability inspired by the
 * formula used in FLAC:
 * relperm = (1 + m) seff^m - m seff^(m + 1)
 */
template <bool is_ad>
class PorousFlowRelativePermeabilityFLACTempl
  : public PorousFlowRelativePermeabilityBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowRelativePermeabilityFLACTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> relativePermeability(GenericReal<is_ad> seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// Exponent m for the specified phase
  const Real _m;
};

typedef PorousFlowRelativePermeabilityFLACTempl<false> PorousFlowRelativePermeabilityFLAC;
typedef PorousFlowRelativePermeabilityFLACTempl<true> ADPorousFlowRelativePermeabilityFLAC;
