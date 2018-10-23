//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObject.h"
#include "MooseObjectAction.h"
#include "MooseUtils.h"
#include "Factory.h"
#include "RelationshipManager.h"
#include "Conversion.h"
#include "MooseMesh.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<MooseObjectAction>()
{
  InputParameters params = validParams<MooseObjectActionBase>();
  params.addRequiredParam<std::string>(
      "type", "A string representing the Moose Object that will be built by this Action");
  params.addParam<bool>("isObjectAction", true, "Indicates that this is a MooseObjectAction.");
  return params;
}

template <>
InputParameters validParams<MooseObject>();

MooseObjectAction::MooseObjectAction(InputParameters params)
  : MooseObjectActionBase(params, params.get<std::string>("type"))
{
}
