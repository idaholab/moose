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
class PorousFlowRelativePermeabilityFLAC : public PorousFlowRelativePermeabilityBase
{
public:
  static InputParameters validParams();

  PorousFlowRelativePermeabilityFLAC(const InputParameters & parameters);

protected:
  virtual Real relativePermeability(Real seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// Exponent m for the specified phase
  const Real _m;
};
