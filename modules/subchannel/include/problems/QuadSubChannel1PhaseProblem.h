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
  virtual Real computeAddedHeatPin(unsigned int i_ch, unsigned int iz) const override;
  virtual Real getSubChannelPeripheralDuctWidth(unsigned int i_ch) const override;
  virtual void computeh(int iblock) override;
  QuadSubChannelMesh & _subchannel_mesh;

public:
  static InputParameters validParams();
};
