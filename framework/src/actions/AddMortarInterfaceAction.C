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
