//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LEVELSETFORCINGFUNCTIONSUPG_H
#define LEVELSETFORCINGFUNCTIONSUPG_H

// MOOSE includes
#include "BodyForce.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
class LevelSetForcingFunctionSUPG;

template <>
InputParameters validParams<LevelSetForcingFunctionSUPG>();

/**
 * SUPG stabilization term for a forcing function.
 */
class LevelSetForcingFunctionSUPG : public LevelSetVelocityInterface<BodyForce>
{
public:
  LevelSetForcingFunctionSUPG(const InputParameters & parameters);

protected:
  Real computeQpResidual() override;
  Real computeQpJacobian() override;
};

#endif // LEVELSETFORCINGFUNCTIONSUPG_H
