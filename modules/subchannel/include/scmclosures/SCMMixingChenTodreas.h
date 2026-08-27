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
 * Class that calculates turbulent mixing and sweep-flow coefficients for wire-wrapped
 * triangular lattices. The user may select either the Cheng-Todreas (1986) or Pacio
 * parameterization.
 */
class SCMMixingChenTodreas : public SCMMixingClosureBase
{
public:
  static InputParameters validParams();

  SCMMixingChenTodreas(const InputParameters & parameters);

  Real computeMixingParameter(const unsigned int i_gap, const unsigned int iz) const override;

  Real computeSweepFlowMixingParameter(const unsigned int i_gap,
                                       const unsigned int iz) const override;

protected:
  /// Keep track of the lattice type
  bool _is_tri_lattice;

  /// Pointer to the triangular lattice mesh
  const TriSubChannelMesh * const _tri_sch_mesh;

  /// Cheng-Todreas mixing-model parameterization
  const MooseEnum & _mixing_model;

  SolutionHandle _S_soln;
  SolutionHandle _mdot_soln;
  SolutionHandle _rho_soln;
};
