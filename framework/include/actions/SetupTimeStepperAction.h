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

#ifndef SETUPTIMESTEPPERACTION_H
#define SETUPTIMESTEPPERACTION_H

#include "MooseObjectAction.h"

class SetupTimeStepperAction;

template <>
InputParameters validParams<SetupTimeStepperAction>();

/**
 *
 */
class SetupTimeStepperAction : public MooseObjectAction
{
public:
  SetupTimeStepperAction(InputParameters parameters);

  virtual void act() override;
};

#endif /* SETUPTIMESTEPPERACTION_H */
