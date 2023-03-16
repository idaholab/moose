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
#include "Conversion.h"
#include "MooseMesh.h"
#include "MooseApp.h"

InputParameters
MooseObjectAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<std::string>(
      "type", "A string representing the Moose Object that will be built by this Action");
  params.addParam<bool>("isObjectAction", true, "Indicates that this is a MooseObjectAction.");
  params.addParamNamesToGroup("isObjectAction", "Advanced");
  params.addClassDescription("Base class for all the actions creating a MOOSE object");
  return params;
}

MooseObjectAction::MooseObjectAction(const InputParameters & params)
  : Action(params),
    _type(getParam<std::string>("type")),
    // We will create a second parameters object from the main factory unless instructed otherwise
    _moose_object_pars(!params.have_parameter<bool>("skip_param_construction") ||
                               (params.have_parameter<bool>("skip_param_construction") &&
                                !params.get<bool>("skip_param_construction"))
                           ? _factory.getValidParams(_type)
                           : MooseObject::validParams())
{
  _moose_object_pars.blockFullpath() = params.blockFullpath();
}

void
MooseObjectAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  addRelationshipManagers(input_rm_type, _moose_object_pars);
}
