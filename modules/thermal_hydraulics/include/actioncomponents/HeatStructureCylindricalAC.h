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
 * Cylindrical heat-conducting solid, as a native ActionComponent.
 *
 * Unlike classic THM's HeatStructureCylindrical (which integrates its own RZ-specific kernels,
 * since general axisymmetric coordinate axes did not yet exist in MOOSE), this uses MOOSE's native
 * COORD_RZ coordinate system with an arbitrarily-positioned/oriented axis, the same mechanism
 * framework/src/actioncomponents/CylinderComponent.C already demonstrates for a 2D RZ
 * ActionComponent - so the inherited HeatStructurePhysics needs no RZ-specific kernels at all.
 *
 * MooseMesh::setCoordSystem()/setGeneralAxisymmetricCoordAxes() must each be called exactly once
 * per simulation, covering every RZ block at once: setCoordSystem() with a single value (even for
 * one block) marks *every* subdomain currently in the mesh as RZ, not just this component's own
 * blocks, and setGeneralAxisymmetricCoordAxes() then requires every RZ-marked block to already
 * have an axis - so calling both once per component (as CylinderComponent does) races whichever
 * heat structure is processed first against the others' blocks being marked RZ before their own
 * axis is set. setupComponent() therefore has whichever instance is constructed first make one
 * combined call covering every HeatStructureCylindricalAC in the simulation; the others do nothing.
 */
class HeatStructureCylindricalAC : public HeatStructureBaseAC
{
public:
  static InputParameters validParams();
  HeatStructureCylindricalAC(const InputParameters & params);

protected:
  virtual Real yOffset() const override { return _inner_radius; }
  virtual void setupComponent() override;

  /// Inner radius of the heat structure
  const Real _inner_radius;
};
