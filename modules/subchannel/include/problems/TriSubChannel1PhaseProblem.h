//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SubChannel1PhaseProblem.h"

class TriSubChannel1PhaseProblem;
class TriSubChannelMesh;
/**
 * Triangular subchannel solver
 */
class TriSubChannel1PhaseProblem : public SubChannel1PhaseProblem
{
public:
  TriSubChannel1PhaseProblem(const InputParameters & params);

  virtual ~TriSubChannel1PhaseProblem();

protected:
  virtual void initializeSolution() override;
  /**
   * Computes the axial friction factor for the sodium coolant and
   * for each subchannel.
   * Upgraded Cheng-Todreas Correlation (2018).
   */
  virtual Real computeFrictionFactor(_friction_args_struct friction_args) override;
  virtual Real computeAddedHeatPin(unsigned int i_ch, unsigned int iz) override;
  virtual void computeWijPrime(int iblock) override;
  virtual void computeh(int iblock) override;
  PetscErrorCode cleanUp();
  TriSubChannelMesh & _tri_sch_mesh;
  // Extra objects for heat conduction, which is important with metal coolants
  Mat _hc_axial_heat_conduction_mat;
  Vec _hc_axial_heat_conduction_rhs;
  Mat _hc_radial_heat_conduction_mat;
  Vec _hc_radial_heat_conduction_rhs;
  Mat _hc_sweep_enthalpy_mat;
  Vec _hc_sweep_enthalpy_rhs;

public:
  static InputParameters validParams();
};
