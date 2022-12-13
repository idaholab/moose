//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatStructureBase.h"
#include "HeatConductionModel.h"

/**
 * Component to model plate heat structure
 */
class HeatStructurePlate : public HeatStructureBase
{
public:
  HeatStructurePlate(const InputParameters & params);

  virtual void check() const override;
  virtual Real getUnitPerimeter(const HeatStructureSideType & side) const override;

protected:
  virtual bool useCylindricalTransformation() const override { return false; }

  /// plate fuel depth
  const Real & _depth;

public:
  static InputParameters validParams();
};
