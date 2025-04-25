//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SubChannel1PhaseProblem.h"
#include "QuadSubChannelMesh.h"

/**
 * Quadrilateral subchannel solver
 */
class QuadSubChannel1PhaseProblem : public SubChannel1PhaseProblem
{
public:
  QuadSubChannel1PhaseProblem(const InputParameters & params);

protected:
  virtual void initializeSolution() override;
  virtual Real computeFrictionFactor(FrictionStruct friction_args) override;
  virtual Real computeAddedHeatPin(unsigned int i_ch, unsigned int iz) override;
  virtual Real computeBeta(unsigned int i_gap, unsigned int iz) override;
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
