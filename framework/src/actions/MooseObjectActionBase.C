//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObject.h"
#include "MooseObjectActionBase.h"
#include "MooseUtils.h"
#include "Factory.h"
#include "RelationshipManager.h"
#include "Conversion.h"
#include "MooseMesh.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<MooseObjectActionBase>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::string>(
      "type", "A string representing the Moose Object that will be built by this Action");
  params.addParam<bool>("isObjectAction", true, "Indicates that this is a MooseObjectAction.");
  return params;
}

template <>
InputParameters validParams<MooseObject>();

MooseObjectActionBase::MooseObjectActionBase(InputParameters params, const std::string & type)
  : Action(params),
    _type(type),
    // We will create a second parameters object from the main factory unless instructed otherwise
    _moose_object_pars(!params.have_parameter<bool>("skip_param_construction") ||
                               (params.have_parameter<bool>("skip_param_construction") &&
                                !params.get<bool>("skip_param_construction"))
                           ? _factory.getValidParams(_type)
                           : validParams<MooseObject>())
{
  _moose_object_pars.blockFullpath() = params.blockFullpath();
}

void
MooseObjectActionBase::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  const auto & buildable_types = _moose_object_pars.getBuildableRelationshipManagerTypes();

  for (const auto & buildable_type : buildable_types)
  {
    auto & rm_name = std::get<0>(buildable_type);
    auto & rm_type = std::get<1>(buildable_type);
    auto rm_input_parameter_func = std::get<2>(buildable_type);

    auto new_name = _moose_object_pars.get<std::string>("_moose_base") + '_' + name() + '_' +
                    rm_name + "_" + Moose::stringify(rm_type);

    auto rm_params = _factory.getValidParams(rm_name);
    rm_params.set<Moose::RelationshipManagerType>("rm_type") = rm_type;

    // Figure out if we shouldn't be adding this one yet
    if (((rm_type & input_rm_type) != input_rm_type) // Does this RM not have the type passed in?

        || // Or are we adding Geometric but this one needs to be delayed

        (((input_rm_type & Moose::RelationshipManagerType::GEOMETRIC) ==
          Moose::RelationshipManagerType::GEOMETRIC) &&
         ((rm_type & Moose::RelationshipManagerType::GEOMETRIC) ==
          Moose::RelationshipManagerType::GEOMETRIC) &&
         !rm_params.template get<bool>("attach_geometric_early"))

        || // Or is this an Algebraic and Geometric one that we already added earlier?

        (((input_rm_type & Moose::RelationshipManagerType::ALGEBRAIC) ==
          Moose::RelationshipManagerType::ALGEBRAIC) &&
         (rm_type == (Moose::RelationshipManagerType::GEOMETRIC |
                      Moose::RelationshipManagerType::ALGEBRAIC)) &&
         rm_params.template get<bool>("attach_geometric_early")))
      continue;

    // If there is a callback for setting the RM parameters let's use it
    if (rm_input_parameter_func)
      rm_input_parameter_func(_moose_object_pars, rm_params);

    // If we're doing geometric but we can't build it early - then let's not build it yet
    // (It will get built when we do algebraic)
    if ((input_rm_type & Moose::RelationshipManagerType::GEOMETRIC) ==
            Moose::RelationshipManagerType::GEOMETRIC &&
        !rm_params.get<bool>("attach_geometric_early"))
    {
      // We also need to tell the mesh not to delete remote elements yet
      // Note this will get reset in AddRelationshipManager::act() when attaching Algebraic
      _mesh->getMesh().allow_remote_element_removal(false);

      // Keep looking for more RMs
      continue;
    }

    rm_params.set<MooseMesh *>("mesh") = _mesh.get();

    if (rm_params.areAllRequiredParamsValid())
    {
      auto rm_obj = _factory.create<RelationshipManager>(rm_name, new_name, rm_params);

      // Delete the resources created on behalf of the RM if it ends up not being added to the App.
      if (!_app.addRelationshipManager(rm_obj))
        _factory.releaseSharedObjects(*rm_obj);
    }
    else
      mooseError("Missing required parameters for RelationshipManager " + rm_name + " for object " +
                 name());
  }
}
