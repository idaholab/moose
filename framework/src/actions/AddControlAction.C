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

#include "AddControlAction.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddControlAction>()
{
  InputParameters params = validParams<AddUserObjectAction>();
  return params;
}

AddControlAction::AddControlAction(InputParameters params) :
  AddUserObjectAction(params)
{
}


void
AddControlAction::act()
{
  std::string base = _moose_object_pars.get<std::string>("_moose_base");

  if (base == "Control")
    AddUserObjectAction::act();
  else if (base == "ControlMaterial")
    _problem->addMaterial(_type, _name, _moose_object_pars);
}
