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

#ifndef ADDINDICATORVARIABLEACTION_H
#define ADDINDICATORVARIABLEACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class AddIndicatorVariableAction;

template<>
InputParameters validParams<AddIndicatorVariableAction>();


class AddIndicatorVariableAction : public Action
{
public:
  AddIndicatorVariableAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif // ADDINDICATORVARIABLEACTION_H
