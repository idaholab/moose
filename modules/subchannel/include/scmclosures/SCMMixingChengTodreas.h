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

/**
 * Class that calculates the turbulent mixing coefficient based on the Cheng & Todreas correlations
 * (Cheng & Todreas 1986). It is used only for wire-wrapped tri lattices. Also, takes care of
 * sweep_flow.
 */
class SCMMixingChengTodreas : public SCMMixingClosureBase
{
public:
  static InputParameters validParams();

  SCMMixingChengTodreas(const InputParameters & parameters);

  Real computeMixingParameter(const unsigned int i_gap,
                              const unsigned int iz,
                              const bool sweep_flow) const override;

  /// Keep track of the lattice type
  bool _is_tri_lattice;
  /// Pointer to the tri lattice mesh
  const TriSubChannelMesh * const _tri_sch_mesh;

  SolutionHandle _S_soln;
  SolutionHandle _mdot_soln;
  SolutionHandle _w_perim_soln;
  SolutionHandle _mu_soln;
};
