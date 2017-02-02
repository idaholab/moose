/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LEVELSETFORCINGFUNCTIONSUPG_H
#define LEVELSETFORCINGFUNCTIONSUPG_H

// MOOSE includes
#include "UserForcingFunction.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
class LevelSetForcingFunctionSUPG;

template<>
InputParameters validParams<LevelSetForcingFunctionSUPG>();

/**
 * SUPG stabilization term for a forcing function.
 */
class LevelSetForcingFunctionSUPG : public LevelSetVelocityInterface<UserForcingFunction>
{
public:

  LevelSetForcingFunctionSUPG(const InputParameters & parameters);

protected:
  Real computeQpResidual() override;
  Real computeQpJacobian() override;
};

#endif // LEVELSETFORCINGFUNCTIONSUPG_H
