//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADHeatFluxFromHeatStructureBaseUserObject.h"

/**
 * Cache the heat flux between a single phase flow channel and a heat structure
 */
class ADHeatFluxFromHeatStructure3EqnUserObject : public ADHeatFluxFromHeatStructureBaseUserObject
{
public:
  ADHeatFluxFromHeatStructure3EqnUserObject(const InputParameters & parameters);

protected:
  virtual ADReal computeQpHeatFlux() override;

  const ADMaterialProperty<Real> & _T_wall;
  const ADMaterialProperty<Real> & _Hw;
  const ADMaterialProperty<Real> & _T;

public:
  static InputParameters validParams();
};
