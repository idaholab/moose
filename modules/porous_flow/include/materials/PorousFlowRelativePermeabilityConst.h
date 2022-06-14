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
 * This class sets a constant relative permeability. This simple class is useful
 * for testing purposes mainly
 */
template <bool is_ad>
class PorousFlowRelativePermeabilityConstTempl
  : public PorousFlowRelativePermeabilityBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowRelativePermeabilityConstTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> relativePermeability(GenericReal<is_ad> seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// Constant relative permeability
  const Real _relperm;
};

typedef PorousFlowRelativePermeabilityConstTempl<false> PorousFlowRelativePermeabilityConst;
typedef PorousFlowRelativePermeabilityConstTempl<true> ADPorousFlowRelativePermeabilityConst;
