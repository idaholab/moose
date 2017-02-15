/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LEVELSETADVECTIONSUPG_H
#define LEVELSETADVECTIONSUPG_H

// MOOSE includes
#include "Kernel.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
class LevelSetAdvectionSUPG;

template<>
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

#endif //LEVELSETADVECTIONSUPG_H
