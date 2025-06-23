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

registerMooseAction("MooseApp", AddGPUMaterialAction, "add_material");

InputParameters
AddGPUMaterialAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a GPUMaterial object to the simulation.");
  return params;
}

AddGPUMaterialAction::AddGPUMaterialAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddGPUMaterialAction::act()
{
#ifndef MOOSE_HAVE_GPU
  mooseError("Attempted to add a GPU material but MOOSE was not compiled with GPU support.");
#else
  if (!_app.hasGPUs())
    mooseError("Attempted to add a GPU material but no GPU was detected in the system.");
  else if (!_moose_object_pars.get<bool>("_interface"))
    _problem->addGPUMaterial(_type, _name, _moose_object_pars);
    // else
    //  _problem->addGPUInterfaceMaterial(_type, _name, _moose_object_pars);
#endif
}
