//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddUserObjectAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddUserObjectAction, "add_user_object");

InputParameters
AddUserObjectAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a UserObject object to the simulation.");
  return params;
}

AddUserObjectAction::AddUserObjectAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddUserObjectAction::act()
{
#ifdef MOOSE_KOKKOS_ENABLED
  if (_moose_object_pars.isParamValid(MooseBase::kokkos_object_param))
    _problem->addKokkosUserObject(_type, _name, _moose_object_pars);
  else
#endif
    _problem->addUserObject(_type, _name, _moose_object_pars);
}
