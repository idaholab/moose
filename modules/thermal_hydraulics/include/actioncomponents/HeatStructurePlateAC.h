//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatStructureBaseAC.h"

/**
 * Plate (Cartesian) heat-conducting solid, as a native ActionComponent.
 */
class HeatStructurePlateAC : public HeatStructureBaseAC
{
public:
  static InputParameters validParams();
  HeatStructurePlateAC(const InputParameters & params);

protected:
  virtual Real yOffset() const override { return 0; }

  /// Dimension of the plate in the third (depth) direction - not yet consumed by any kernel or
  /// postprocessor in this geometry+physics-only implementation; reserved for the coupling/
  /// postprocessor work that classic THM uses it for (see HeatStructurePlate::getUnitPerimeter()).
  const Real _depth;
};
