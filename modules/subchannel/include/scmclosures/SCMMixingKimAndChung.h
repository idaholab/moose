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

class SubChannelMesh;

/**
 * Class that calculates the turbulent mixing coefficient based on the Kim and Chung (2001)
 * correllation (eq 25). It is used for both quad and tri lattices with bare fuel pins.
 */
class SCMMixingKimAndChung : public SCMMixingClosureBase
{
public:
  static InputParameters validParams();

  SCMMixingKimAndChung(const InputParameters & parameters);

  virtual Real computeMixingParameter(const unsigned int i_gap,
                                      const unsigned int iz,
                                      const bool sweep_flow) const override;

protected:
  Real computeTriLatticeMixingParameter(const unsigned int i_gap,
                                        const unsigned int iz,
                                        const bool sweep_flow) const;

  Real computeQuadLatticeMixingParameter(const unsigned int i_gap,
                                         const unsigned int iz,
                                         const bool sweep_flow) const;

  Real computeLatticeMixingParameter(const unsigned int i_gap,
                                     const unsigned int iz,
                                     const Real delta,
                                     const Real sf,
                                     const bool sweep_flow) const;

  /// Keep track of the lattice type
  bool _is_tri_lattice;
  /// Pointer to the base subchannel mesh
  const SubChannelMesh & _sch_mesh;

  SolutionHandle _S_soln;
  SolutionHandle _mdot_soln;
  SolutionHandle _w_perim_soln;
  SolutionHandle _mu_soln;
  SolutionHandle _P_soln;
  SolutionHandle _T_soln;
};
