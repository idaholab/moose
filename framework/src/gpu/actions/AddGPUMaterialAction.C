//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddGPUMaterialAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddKokkosMaterialAction, "add_material");

InputParameters
AddKokkosMaterialAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Kokkos Material object to the simulation.");
  params.addPrivateParam<bool>("_kokkos_action", true);
  return params;
}

AddKokkosMaterialAction::AddKokkosMaterialAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddKokkosMaterialAction::act()
{
#ifndef MOOSE_HAVE_KOKKOS
  mooseError("Attempted to add a Kokkos material but MOOSE was not compiled with Kokkos support.");
#else
  if (!_app.hasGPUs())
    mooseError("Attempted to add a Kokkos material but no GPU was detected in the system.");
  else if (!_moose_object_pars.get<bool>("_interface"))
    _problem->addKokkosMaterial(_type, _name, _moose_object_pars);
    // else
    //  _problem->addKokkosInterfaceMaterial(_type, _name, _moose_object_pars);
#endif
}
