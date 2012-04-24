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

#ifndef ADDUSERDATAACTION_H
#define ADDUSERDATAACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObjectAction.h"

#include <string>

class AddUserDataAction;

template<>
InputParameters validParams<AddUserDataAction>();


class AddUserDataAction : public MooseObjectAction
{
public:
  AddUserDataAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif // ADDUSERDATAACTION_H
