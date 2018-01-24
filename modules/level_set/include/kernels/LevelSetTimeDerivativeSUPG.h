//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LEVELSETTIMEDERIVATIVESUPG_H
#define LEVELSETTIMEDERIVATIVESUPG_H

// MOOSE includes
#include "TimeDerivative.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
class LevelSetTimeDerivativeSUPG;

template <>
InputParameters validParams<LevelSetTimeDerivativeSUPG>();

/**
 * Applies SUPG stabilization to the time derivative.
 */
class LevelSetTimeDerivativeSUPG : public LevelSetVelocityInterface<TimeDerivative>
{
public:
  LevelSetTimeDerivativeSUPG(const InputParameters & parameters);

protected:
  Real computeQpResidual() override;
  Real computeQpJacobian() override;
};

#endif // LEVELSETTIMEDERIVATIVESUPG_H
