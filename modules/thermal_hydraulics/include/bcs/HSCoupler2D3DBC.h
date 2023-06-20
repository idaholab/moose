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

class HSCoupler2D3DUserObject;

/**
 * Adds heat flux terms for HSCoupler2D3D
 */
class HSCoupler2D3DBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  HSCoupler2D3DBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// HSCoupler2D3D user object
  const HSCoupler2D3DUserObject & _hs_coupler_2d3d_uo;
};
