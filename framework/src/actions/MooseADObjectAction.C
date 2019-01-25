//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObject.h"
#include "MooseADObjectAction.h"
#include "MooseUtils.h"
#include "Factory.h"
#include "RelationshipManager.h"
#include "Conversion.h"
#include "MooseMesh.h"
#include "MooseApp.h"
#include "FEProblemBase.h"

template <>
InputParameters
validParams<MooseADObjectAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::string>(
      "type", "A string representing the Moose Object that will be built by this Action");
  params.addParam<bool>("isObjectAction", true, "Indicates that this is a MooseADObjectAction.");
  return params;
}

template <>
InputParameters validParams<MooseObject>();

MooseADObjectAction::MooseADObjectAction(InputParameters params)
  : Action(params),
    _base_type(getParam<std::string>("type")),
    _type(_base_type+"<RESIDUAL>"),
    // We will create a second parameters object from the main factory unless instructed otherwise
    _moose_object_pars(!params.have_parameter<bool>("skip_param_construction") ||
                               (params.have_parameter<bool>("skip_param_construction") &&
                                !params.get<bool>("skip_param_construction"))
                           ? _factory.getValidParams(_type)
                           : validParams<MooseObject>())
{
  _moose_object_pars.blockFullpath() = params.blockFullpath();
}

void MooseADObjectAction::addRelationshipManagers(Moose::RelationshipManagerType /*when_type*/)
{
  const auto & buildable_types = _moose_object_pars.getBuildableRelationshipManagerTypes();

  for (const auto & buildable_type : buildable_types)
  {
    /**
     * This method is always called twice. Once to attempt adding early RMs and once to add late
     * RMs. For generic MooseObjects, we'd like to add RMs as early as possible, but we'll have to
     * be careful not to add them twice!
     */
    auto new_name = name() + '_' + buildable_type.first + "_rm";
    if (_app.hasRelationshipManager(new_name))
      continue;

    auto rm_params = _factory.getValidParams(buildable_type.first);
    rm_params.applyParameters(_moose_object_pars);
    rm_params.set<MooseMesh *>("mesh") = _mesh.get();
    rm_params.set<Moose::RelationshipManagerType>("rm_type") = buildable_type.second;

    if (rm_params.areAllRequiredParamsValid())
    {
      auto rm_obj = _factory.create<RelationshipManager>(buildable_type.first, new_name, rm_params);

      // Delete the resources created on behalf of the RM if it ends up not being added to the App.
      if (!_app.addRelationshipManager(rm_obj))
        _factory.releaseSharedObjects(*rm_obj);
    }
  }
}

void
MooseADObjectAction::flagDoingAD()
{
  _problem->haveADObjects(true);
}
