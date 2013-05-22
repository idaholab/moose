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

template<>
InputParameters validParams<SetupTimeStepperAction>();

/**
 *
 */
class SetupTimeStepperAction : public MooseObjectAction
{
public:
  SetupTimeStepperAction(const std::string & name, InputParameters parameters);
  virtual ~SetupTimeStepperAction();

  virtual void act();

protected:

};


#endif /* SETUPTIMESTEPPERACTION_H */
