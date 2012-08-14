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

#include "AddCoupledVariableAction.h"
#include "CoupledExecutioner.h"
#include "Parser.h"


template<>
InputParameters validParams<AddCoupledVariableAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::string>("from", "The name of the problem we are getting the variable from");
  params.addRequiredParam<std::string>("var_name", "The name of the variable we are getting from the problem");
  return params;
}

AddCoupledVariableAction::AddCoupledVariableAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _from(getParam<std::string>("from")),
    _var_name(getParam<std::string>("var_name"))
{
}

AddCoupledVariableAction::~AddCoupledVariableAction()
{
}

void
AddCoupledVariableAction::act()
{
  CoupledExecutioner * master_executioner = dynamic_cast<CoupledExecutioner *>(_executioner);
  if (master_executioner != NULL)
  {
    std::vector<std::string> parts;
    Parser::tokenize(name(), parts, 0, "/");
    if (parts.size() > 2)
      // parts[size - 2] is the one before last, i.e. the name of the destination problem
      master_executioner->addCoupledVariable(parts[parts.size() - 2], getShortName(), _from, _var_name);
    else
      mooseError("Weird syntax detected.");
  }
}
