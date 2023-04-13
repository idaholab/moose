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

/**
 * Base class for cylindrical heat structure components
 */
class HeatStructureCylindricalBase : public HeatStructureBase
{
public:
  HeatStructureCylindricalBase(const InputParameters & params);

  virtual void setupMesh() override;
  virtual Real getUnitPerimeter(const ExternalBoundaryType & side) const override;

  /**
   * Get the inner radius of the heat structure
   *
   * @returns The inner radius of the heat structure
   */
  virtual Real getInnerRadius() const { return _inner_radius; }

  virtual Real computeRadialBoundaryArea(const Real & length, const Real & y) const override;
  virtual Real computeAxialBoundaryArea(const Real & y_min, const Real & y_max) const override;

protected:
  virtual bool useCylindricalTransformation() const override { return true; }

  /// Inner radius of the heat structure
  Real _inner_radius;

public:
  static InputParameters validParams();
};
