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

#ifndef ADDSTEPPERACTION_H
#define ADDSTEPPERACTION_H

#include "MooseObjectAction.h"

//Forward Declaration
class AddStepperAction;

template<>
InputParameters validParams<AddStepperAction>();


class AddStepperAction : public MooseObjectAction
{
public:
  AddStepperAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDSTEPPERACTION_H
