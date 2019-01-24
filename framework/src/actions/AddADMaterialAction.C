//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddADMaterialAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddADMaterialAction, "add_ad_material");

template <>
InputParameters
validParams<AddADMaterialAction>()
{
  InputParameters params = validParams<MooseADObjectAction>();
  params.addClassDescription(
      "This action is used to add ADMaterial<RESIDUAL> and ADMaterial<JACOBIAN> objects");
  return params;
}

AddADMaterialAction::AddADMaterialAction(InputParameters params) : MooseADObjectAction(params) {}

void
AddADMaterialAction::act()
{
  flagDoingAD();
  _problem->addADResidualMaterial(_type, _name, _moose_object_pars);
  std::string to_erase = "<RESIDUAL>";
  std::string::size_type match = _type.find(to_erase);
  if (match != std::string::npos)
    _type.erase(match, to_erase.length());
  _problem->addADJacobianMaterial(_type + "<JACOBIAN>", _name + "_jacobian", _moose_object_pars);
}
