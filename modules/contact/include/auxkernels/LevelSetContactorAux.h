//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class LevelSetContactor;

/**
 * Sample a LevelSetContactor at nodal or elemental locations into an aux
 * variable.  Useful for visualization of the SDF field and for regression
 * tests that compare mesh-based contactors against analytic ones.
 */
class LevelSetContactorAux : public AuxKernel
{
public:
  static InputParameters validParams();
  LevelSetContactorAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  const LevelSetContactor & _contactor;
  const MooseEnum _quantity;
};
