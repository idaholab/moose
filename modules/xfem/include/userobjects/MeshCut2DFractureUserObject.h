//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshCut2DUserObjectBase.h"

class CrackFrontDefinition;

/**
 * MeshCut2DFractureUserObject:
 * (1) uses the mesh to do initial cutting of 2D elements, and
 * (2) grows the mesh by a fixed growth rate.
 */

class MeshCut2DFractureUserObject : public MeshCut2DUserObjectBase
{
public:
  static InputParameters validParams();

  MeshCut2DFractureUserObject(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;

protected:
  virtual void findActiveBoundaryGrowth() override;

private:
  const Real _k_critical_squared;
  const Real _growth_increment;

  CrackFrontDefinition * _crack_front_definition;

  /// compute k_squared from fracture integrals
  std::vector<Real> getKSquared(const std::vector<Real> & k1, const std::vector<Real> & k2) const;
};
