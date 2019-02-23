//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMaterialAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddMaterialAction, "add_material");

template <>
InputParameters
validParams<AddMaterialAction>()
{
  return validParams<MooseObjectAction>();
}

AddMaterialAction::AddMaterialAction(InputParameters params) : MooseObjectAction(params) {}

void
AddMaterialAction::act()
{
  if (Registry::isADObj(_type + "<RESIDUAL>"))
  {
    _problem->addADResidualMaterial(_type + "<RESIDUAL>", _name + "_residual", _moose_object_pars);
    _problem->addADJacobianMaterial(_type + "<JACOBIAN>", _name + "_jacobian", _moose_object_pars);
    _problem->haveADObjects(true);
  }
  else
    _problem->addMaterial(_type, _name, _moose_object_pars);
}
