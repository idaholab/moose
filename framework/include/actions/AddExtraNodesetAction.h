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

#ifndef ADDEXTRANODESETACTION_H
#define ADDEXTRANODESETACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

//Forward Declaration
class MooseMesh;

class AddExtraNodesetAction;

template<>
InputParameters validParams<AddExtraNodesetAction>();


class AddExtraNodesetAction : public Action
{
public:
  AddExtraNodesetAction(const std::string & name, InputParameters params);
  
  virtual void act();
};

#endif // ADDEXTRANODESETACTION_H
