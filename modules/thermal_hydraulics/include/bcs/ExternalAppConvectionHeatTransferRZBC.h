//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExternalAppConvectionHeatTransferBC.h"
#include "RZSymmetry.h"

/**
 * Convection BC from an external application for RZ domain in XY coordinate system
 */
class ExternalAppConvectionHeatTransferRZBC : public ExternalAppConvectionHeatTransferBC,
                                              public RZSymmetry
{
public:
  ExternalAppConvectionHeatTransferRZBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

public:
  static InputParameters validParams();
};
