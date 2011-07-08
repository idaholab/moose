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

#ifndef SETUPPRECONDITIONERACTION_H
#define SETUPPRECONDITIONERACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObjectAction.h"

#include <string>

class SetupPreconditionerAction : public MooseObjectAction
{
public:
  SetupPreconditionerAction(const std::string & name, InputParameters params);

  virtual void act();
  
protected:
  static unsigned int _count;
  static std::map<std::string, std::string> _type_to_action;
};

template<>
InputParameters validParams<SetupPreconditionerAction>();

#endif // SETUPPRECONDITIONERACTION_H
