/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LEVELSETADVECTION_H
#define LEVELSETADVECTION_H

// MOOSE includes
#include "Kernel.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
class LevelSetAdvection;

template<>
InputParameters validParams<LevelSetAdvection>();

/**
 * Advection Kernel for the levelset equation.
 *
 * \psi_i \vec{v} \nabla u,
 * where \vec{v} is the interface velocity that is a set of
 * coupled variables.
 */
class LevelSetAdvection :
  public LevelSetVelocityInterface<Kernel>
{
public:

  LevelSetAdvection(const InputParameters & parameters);

protected:

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
};

#endif //LEVELSETADVECTION_H
