/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LEVELSETTIMEDERIVATIVESUPG_H
#define LEVELSETTIMEDERIVATIVESUPG_H

// MOOSE includes
#include "TimeDerivative.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
class LevelSetTimeDerivativeSUPG;

template<>
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

#endif //LEVELSETTIMEDERIVATIVESUPG_H
