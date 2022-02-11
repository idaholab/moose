//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RadiativeHeatFluxBC.h"
#include "RZSymmetry.h"

/**
 * Radiative heat transfer boundary condition for a cylindrical heat structure
 */
class RadiativeHeatFluxRZBC : public RadiativeHeatFluxBC, public RZSymmetry
{
public:
  RadiativeHeatFluxRZBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

public:
  static InputParameters validParams();
};
