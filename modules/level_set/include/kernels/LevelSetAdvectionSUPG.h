//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LEVELSETADVECTIONSUPG_H
#define LEVELSETADVECTIONSUPG_H

// MOOSE includes
#include "Kernel.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
class LevelSetAdvectionSUPG;

template <>
InputParameters validParams<LevelSetAdvectionSUPG>();

/**
 * SUPG stabilization for the advection portion of the level set equation.
 */
class LevelSetAdvectionSUPG : public LevelSetVelocityInterface<Kernel>
{
public:
  LevelSetAdvectionSUPG(const InputParameters & parameters);

protected:
  Real computeQpResidual() override;
  Real computeQpJacobian() override;
};

#endif // LEVELSETADVECTIONSUPG_H
