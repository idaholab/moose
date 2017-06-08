/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DARCYVELOCITYACTION_H
#define DARCYVELOCITYACTION_H

#include "Action.h"

class DarcyVelocityAction;

template <>
InputParameters validParams<DarcyVelocityAction>();

/**
 * An action for creating AuxVariables and AuxKernels for Darcy Velocity Calculation
 */
class DarcyVelocityAction : public Action
{
public:
  DarcyVelocityAction(InputParameters parameters);
  virtual void act() override;
};
#endif
