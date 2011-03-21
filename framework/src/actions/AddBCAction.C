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

#include "AddBCAction.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddBCAction>()
{
  return validParams<MooseObjectAction>();
}

AddBCAction::AddBCAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddBCAction::act()
{
  if (Parser::pathContains(_name, "BCs"))
    _parser_handle._problem->addBoundaryCondition(_type, getShortName(), _moose_object_pars);
  else
    _parser_handle._problem->addAuxBoundaryCondition(_type, getShortName(), _moose_object_pars);  
}
