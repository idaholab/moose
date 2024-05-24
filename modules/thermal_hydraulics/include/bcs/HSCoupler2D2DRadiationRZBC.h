//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"
#include "RZSymmetry.h"

class HSCoupler2D2DRadiationUserObject;

/**
 * Adds heat flux terms for HSCoupler2D2DRadiation
 */
class HSCoupler2D2DRadiationRZBC : public ADIntegratedBC, public RZSymmetry
{
public:
  static InputParameters validParams();

  HSCoupler2D2DRadiationRZBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// HSCoupler2D2DRadiation user object
  const HSCoupler2D2DRadiationUserObject & _hs_coupler_2d2d_uo;
};
