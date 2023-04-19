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
  virtual Real getUnitPerimeter(const ExternalBoundaryType & side) const override;

  /**
   * Gets the depth of the plate
   */
  const Real & getDepth() const { return _depth; }

  virtual Real computeRadialBoundaryArea(const Real & length, const Real & y) const override;
  virtual Real computeAxialBoundaryArea(const Real & y_min, const Real & y_max) const override;

protected:
  virtual bool useCylindricalTransformation() const override { return false; }

  /// plate fuel depth
  const Real & _depth;

public:
  static InputParameters validParams();
};
