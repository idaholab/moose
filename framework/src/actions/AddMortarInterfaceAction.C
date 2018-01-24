//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AddMortarInterfaceAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<AddMortarInterfaceAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<BoundaryName>("master", "Master side ID");
  params.addRequiredParam<BoundaryName>("slave", "Slave side ID");
  params.addRequiredParam<SubdomainName>("subdomain",
                                         "Subdomain name that is the mortar interface");

  return params;
}

AddMortarInterfaceAction::AddMortarInterfaceAction(InputParameters parameters) : Action(parameters)
{
}

void
AddMortarInterfaceAction::act()
{
  std::string iface_name = name();

  _mesh->addMortarInterface(iface_name,
                            getParam<BoundaryName>("master"),
                            getParam<BoundaryName>("slave"),
                            getParam<SubdomainName>("subdomain"));
  if (_displaced_mesh)
    _displaced_mesh->addMortarInterface(iface_name,
                                        getParam<BoundaryName>("master"),
                                        getParam<BoundaryName>("slave"),
                                        getParam<SubdomainName>("subdomain"));
}
