//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetAdaptivityOptionsAction.h"
#include "FEProblem.h"
#include "RelationshipManager.h"

#include "libmesh/fe.h"

registerMooseAction("MooseApp", SetAdaptivityOptionsAction, "set_adaptivity_options");
registerMooseAction("MooseApp", SetAdaptivityOptionsAction, "add_geometric_rm");
registerMooseAction("MooseApp", SetAdaptivityOptionsAction, "add_algebraic_rm");

InputParameters
SetAdaptivityOptionsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Action for defining adaptivity parameters.");
  params.addParam<MarkerName>("marker",
                              "The name of the Marker to use to actually adapt the mesh.");
  params.addParam<unsigned int>(
      "steps", 0, "The number of adaptive steps to use when doing a Steady simulation.");
  params.addParam<unsigned int>(
      "initial_steps", 0, "The number of adaptive steps to do based on the initial condition.");
  params.addRangeCheckedParam<unsigned int>(
      "interval", 1, "interval>0", "The number of time steps betweeen each adaptivity phase");
  params.addParam<MarkerName>(
      "initial_marker",
      "The name of the Marker to use to adapt the mesh during initial refinement.");
  params.addParam<unsigned int>(
      "max_h_level",
      0,
      "Maximum number of times a single element can be refined. If 0 then infinite.");
  params.addParam<Real>("start_time",
                        -std::numeric_limits<Real>::max(),
                        "The time that adaptivity will be active after.");
  params.addParam<Real>("stop_time",
                        std::numeric_limits<Real>::max(),
                        "The time after which adaptivity will no longer be active.");
  params.addParam<unsigned int>(
      "cycles_per_step",
      1,
      "The number of adaptive steps to use when on each timestep during a Transient simulation.");
  params.addParam<bool>(
      "recompute_markers_during_cycles", false, "Recompute markers during adaptivity cycles");
  return params;
}

SetAdaptivityOptionsAction::SetAdaptivityOptionsAction(const InputParameters & params)
  : Action(params)
{
}

void
SetAdaptivityOptionsAction::act()
{
  // Here we are going to mostly mimic the default ghosting in libmesh
  // By default libmesh adds:
  // 1) GhostPointNeighbors on the mesh
  // 2) DefaultCoupling with 1 layer as an algebraic ghosting functor on the dof_map, which also
  //    gets added to the mesh at the time a new System is added
  // 3) DefaultCoupling with 0 layers as a coupling functor on the dof_map, which also gets added to
  //    the mesh at the time a new System is added
  //
  // What we will do differently is:
  // - The 3rd ghosting functor adds nothing so we will not add it at all

  if (_current_task == "add_algebraic_rm")
  {
    auto rm_params = _factory.getValidParams("ElementSideNeighborLayers");

    rm_params.set<std::string>("for_whom") = "Adaptivity";
    rm_params.set<MooseMesh *>("mesh") = _mesh.get();
    rm_params.set<Moose::RelationshipManagerType>("rm_type") =
        Moose::RelationshipManagerType::ALGEBRAIC;

    if (rm_params.areAllRequiredParamsValid())
    {
      auto rm_obj = _factory.create<RelationshipManager>(
          "ElementSideNeighborLayers", "adaptivity_algebraic_ghosting", rm_params);

      // Delete the resources created on behalf of the RM if it ends up not being added to the
      // App.
      if (!_app.addRelationshipManager(rm_obj))
        _factory.releaseSharedObjects(*rm_obj);
    }
    else
      mooseError("Invalid initialization of ElementSideNeighborLayers");
  }

  else if (_current_task == "add_geometric_rm")
  {
    auto rm_params = _factory.getValidParams("MooseGhostPointNeighbors");

    rm_params.set<std::string>("for_whom") = "Adaptivity";
    rm_params.set<MooseMesh *>("mesh") = _mesh.get();
    rm_params.set<Moose::RelationshipManagerType>("rm_type") =
        Moose::RelationshipManagerType::GEOMETRIC;

    if (rm_params.areAllRequiredParamsValid())
    {
      auto rm_obj = _factory.create<RelationshipManager>(
          "MooseGhostPointNeighbors", "adaptivity_geometric_ghosting", rm_params);

      // Delete the resources created on behalf of the RM if it ends up not being added to the
      // App.
      if (!_app.addRelationshipManager(rm_obj))
        _factory.releaseSharedObjects(*rm_obj);
    }
    else
      mooseError("Invalid initialization of MooseGhostPointNeighbors");
  }

  else if (_current_task == "set_adaptivity_options")
  {
    Adaptivity & adapt = _problem->adaptivity();

    if (isParamValid("marker"))
      adapt.setMarkerVariableName(getParam<MarkerName>("marker"));
    if (isParamValid("initial_marker"))
      adapt.setInitialMarkerVariableName(getParam<MarkerName>("initial_marker"));

    adapt.setCyclesPerStep(getParam<unsigned int>("cycles_per_step"));

    adapt.setMaxHLevel(getParam<unsigned int>("max_h_level"));

    adapt.init(getParam<unsigned int>("steps"), getParam<unsigned int>("initial_steps"));
    adapt.setUseNewSystem();

    adapt.setTimeActive(getParam<Real>("start_time"), getParam<Real>("stop_time"));
    adapt.setInterval(getParam<unsigned int>("interval"));

    adapt.setRecomputeMarkersFlag(getParam<bool>("recompute_markers_during_cycles"));
  }
}
