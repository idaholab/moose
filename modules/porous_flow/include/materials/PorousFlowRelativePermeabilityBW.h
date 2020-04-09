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
#include "PorousFlowBroadbridgeWhite.h"

/**
 * Material that calculates the Broadbridge-White relative permeability
 * P Broadbridge, I White ``Constant rate rainfall
 * infiltration: A versatile nonlinear model, 1 Analytical solution''.
 * Water Resources Research 24 (1988) 145--154.
 */
class PorousFlowRelativePermeabilityBW : public PorousFlowRelativePermeabilityBase
{
public:
  static InputParameters validParams();

  PorousFlowRelativePermeabilityBW(const InputParameters & parameters);

protected:
  virtual Real relativePermeability(Real seff) const override;
  virtual Real dRelativePermeability(Real seff) const override;

  /// BW's low saturation
  const Real _sn;

  /// BW's high saturation
  const Real _ss;

  /// BW's low relative permeability
  const Real _kn;

  /// BW's high relative permeability
  const Real _ks;

  /// BW's C parameter
  const Real _c;
};
