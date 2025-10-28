//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SCMFrictionClosureBase.h"
#include "QuadSubChannelMesh.h"

/**
 * Class that calculates the friction factor based on the Bo Pang correlations
 * (Bo Pang. 2018). It is used for quad lattices.
 */
class SCMFrictionBoPang : public SCMFrictionClosureBase
{
public:
  static InputParameters validParams();

  SCMFrictionBoPang(const InputParameters & parameters);

  virtual Real computeFrictionFactor(const FrictionStruct & friction_info) const override;

protected:
  Real computeQuadLatticeFrictionFactor(const FrictionStruct & friction_info) const;

  /// Keep track of the lattice type
  bool _is_quad_lattice;
  /// Pointer to the quad lattice mesh
  const QuadSubChannelMesh * const _quad_sch_mesh;
};
