//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Postprocessor.h"
#include "UserObject.h"

template <>
InputParameters
validParams<Postprocessor>()
{
  InputParameters params = validParams<UserObject>();
  params += validParams<OutputInterface>();

  params.addParamNamesToGroup("outputs", "Advanced");
  params.registerBase("Postprocessor");
  return params;
}

Postprocessor::Postprocessor(const InputParameters & parameters)
  : OutputInterface(parameters), _pp_name(parameters.get<std::string>("_object_name"))
{
}
