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

#include "AddMeshModifierAction.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddMeshModifierAction>()
{
  return validParams<MooseObjectAction>();
}

AddMeshModifierAction::AddMeshModifierAction(const std::string & name, InputParameters params) :
  MooseObjectAction(name, params)
{
}

void
AddMeshModifierAction::act()
{
  _parser_handle._mesh->addMeshModifer(_type, getShortName(), _moose_object_pars);
}

