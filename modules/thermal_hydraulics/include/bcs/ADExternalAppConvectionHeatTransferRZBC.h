//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADExternalAppConvectionHeatTransferBC.h"
#include "RZSymmetry.h"

/**
 * Convection BC from an external application for RZ domain in XY coordinate system
 */
class ADExternalAppConvectionHeatTransferRZBC : public ADExternalAppConvectionHeatTransferBC,
                                                public RZSymmetry
{
public:
  ADExternalAppConvectionHeatTransferRZBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

public:
  static InputParameters validParams();
};
