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

#include "MooseObjectAction.h"
#include "MooseUtils.h"
#include "Factory.h"
#include "RelationshipManager.h"
#include "Conversion.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<MooseObjectAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::string>(
      "type", "A string representing the Moose Object that will be built by this Action");
  params.addParam<bool>("isObjectAction", true, "Indicates that this is a MooseObjectAction.");
  return params;
}

MooseObjectAction::MooseObjectAction(InputParameters params)
  : Action(params),
    _type(getParam<std::string>("type")),
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
MooseObjectAction::addRelationshipManagers(Moose::RelationshipManagerType rm_type)
{
  const auto & buildable_types = _moose_object_pars.getBuildableRelationshipManagerTypes();

  for (const auto & buildable_type : buildable_types)
  {
    auto rm_params = _factory.getValidParams(buildable_type);
    mooseAssert(rm_params.isParamValid("RelationshipManagerType"),
                "RelationshipManagerType is not set for " + buildable_type);

    if (rm_type != rm_params.get<Moose::RelationshipManagerType>("RelationshipManagerType"))
      continue;

    rm_params.applyParameters(_moose_object_pars);
    rm_params.set<MooseMesh *>("mesh") = _mesh.get();

    if (rm_params.areAllRequiredParamsValid())
    {
      auto rm_obj = _factory.create<RelationshipManager>(
          buildable_type, name() + "_rm" + Moose::stringify(rm_type), rm_params);

      rm_obj->init();

      if (rm_obj->isActive())
      {
        if (rm_type == Moose::Geometric)
          // Hand off ownership of the shared pointer to the Mesh
          _mesh->addRelationshipManager(std::move(rm_obj));
        //      else
        //        _problem->getNonlinearSystemBase().dofMap().add_coupling_functor(std::move(rm_obj));
      }
    }
  }
}
