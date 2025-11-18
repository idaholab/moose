//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SCMMixingClosureBase.h"
#include "TriSubChannelMesh.h"
#include "QuadSubChannelMesh.h"

/**
 * Class that calculates the mixing parameter based on the Kim and Chung (2001) corellation (eq 25)
 * It is used for both quad and tri lattices with bare fuel pins.
 */
class SCMMixingKimAndChung : public SCMMixingClosureBase
{
public:
  static InputParameters validParams();

  SCMMixingKimAndChung(const InputParameters & parameters);

  virtual Real computeMixingParameter(const unsigned int & i_gap,
                                      const unsigned int & iz) const override;

protected:
  Real computeTriLatticeMixingParameter(const unsigned int & i_gap, const unsigned int & iz) const;
  Real computeQuadLatticeMixingParameter(const unsigned int & i_gap, const unsigned int & iz) const;

  /// Keep track of the lattice type
  bool _is_tri_lattice;
  /// Pointer to the tri lattice mesh
  const TriSubChannelMesh * const _tri_sch_mesh;
  /// Pointer to the quad lattice mesh
  const QuadSubChannelMesh * const _quad_sch_mesh;
};
