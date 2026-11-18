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

  void computeBulkMixingParameters() const override;

  void computeBlockMixingParameters(const unsigned int first_node,
                                    const unsigned int last_node) const override;

protected:
  /// Pacio mixing parameter of a gap between subchannels with flow splits Xi and Xj, including
  /// the ratio of the Pacio contact perimeter per gap to the gap width
  Real computePacioMixingParameter(const Real Xi, const Real Xj, const Real perimeter_ratio) const;

  /// Keep track of the lattice type
  bool _is_tri_lattice;

  /// Pointer to the triangular lattice mesh
  const TriSubChannelMesh * const _tri_sch_mesh;

  /// Chen-Todreas mixing-model parameterization
  const MooseEnum & _mixing_model;

  SolutionHandle _S_soln;
  SolutionHandle _mdot_soln;
  SolutionHandle _rho_soln;

  /// Cheng-Todreas (1986) mixing parameter of the gaps next to a center subchannel, which depends
  /// only on the geometry and the bulk Reynolds number
  mutable Real _beta_1986;

  /// Cheng-Todreas (1986) sweep-flow coefficient of the peripheral gaps, which depends only on the
  /// geometry and the bulk Reynolds number
  mutable Real _beta_sweep;

  /// Pacio mixing parameter of the center-edge and edge-corner gaps per axial level, from the
  /// flow split of each subchannel type lumped over the axial level
  mutable std::vector<Real> _beta_pacio_center_edge;
  mutable std::vector<Real> _beta_pacio_edge_corner;
};
