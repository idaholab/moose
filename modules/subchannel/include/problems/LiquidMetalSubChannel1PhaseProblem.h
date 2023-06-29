/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "SubChannel1PhaseProblem.h"

class LiquidMetalSubChannel1PhaseProblem;
class TriSubChannelMesh;
/**
 * Steady state subchannel solver for 1-phase hex liquid metal coolants
 */
class LiquidMetalSubChannel1PhaseProblem : public SubChannel1PhaseProblem
{
public:
  LiquidMetalSubChannel1PhaseProblem(const InputParameters & params);

  virtual ~LiquidMetalSubChannel1PhaseProblem();

protected:
  /**
   * Computes the axial friction factor for the sodium coolant and
   * for each subchannel.
   * Upgraded Cheng-Todreas Correlation (2018).
   */
  virtual Real computeFrictionFactor(_friction_args_struct friction_args) override;
  virtual Real computeAddedHeatPin(unsigned int i_ch, unsigned int iz) override;
  virtual void computeWijPrime(int iblock) override;
  virtual void computeh(int iblock) override;
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
