//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddComponentAction.h"

registerMooseAction("ThermalHydraulicsApp", AddComponentAction, "THM:add_component");

InputParameters
AddComponentAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params += THMAppInterface::validParams();

  params.makeParamNotRequired<std::string>("type");
  params.set<std::string>("type") = "ComponentGroup";

  return params;
}

AddComponentAction::AddComponentAction(const InputParameters & params)
  : MooseObjectAction(params), THMAppInterface(params), _group(_type == "ComponentGroup")
{
}

void
AddComponentAction::act()
{
  if (!_group)
  {
    // get the component name
    std::vector<std::string> parts;
    MooseUtils::tokenize<std::string>(_moose_object_pars.blockFullpath(), parts);

    std::stringstream res;
    std::copy(++parts.begin(), parts.end(), std::ostream_iterator<std::string>(res, "/"));

    std::string comp_name = res.str();
    comp_name.pop_back();

    // add the component
    getTHMApp().addComponent(_type, comp_name, _moose_object_pars);
  }
}
