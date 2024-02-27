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
#include "QuadSubChannelMesh.h"

class QuadSubChannel1PhaseProblem;
/**
 * Steady state subchannel solver for 1-phase quad liquid water coolant
 */
class QuadSubChannel1PhaseProblem : public SubChannel1PhaseProblem
{
public:
  QuadSubChannel1PhaseProblem(const InputParameters & params);

protected:
  virtual void initializeSolution() override;
  virtual Real computeFrictionFactor(_friction_args_struct friction_args) override;
  virtual Real computeAddedHeatPin(unsigned int i_ch, unsigned int iz) override;
  virtual void computeWijPrime(int iblock) override;
  virtual void computeh(int iblock) override;
  QuadSubChannelMesh & _subchannel_mesh;

  /// Thermal diffusion coefficient used in turbulent crossflow
  const Real & _beta;
  /// Flag that activates one of the two friction models (default: f=a*Re^b, non-default: Todreas-Kazimi)
  const bool _default_friction_model;
  /// Flag that activates the use of constant beta
  const bool _constant_beta;

public:
  static InputParameters validParams();
};
