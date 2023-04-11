//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptivityAction.h"

#ifdef LIBMESH_ENABLE_AMR

#include "FEProblem.h"
#include "NonlinearSystemBase.h"
#include "Adaptivity.h"
#include "Executioner.h"
#include "MooseEnum.h"
#include "MooseVariableFE.h"
#include "RelationshipManager.h"

// libMesh includes
#include "libmesh/transient_system.h"
#include "libmesh/system_norm.h"
#include "libmesh/enum_norm_type.h"

registerMooseAction("MooseApp", AdaptivityAction, "setup_adaptivity");
registerMooseAction("MooseApp", AdaptivityAction, "add_geometric_rm");
registerMooseAction("MooseApp", AdaptivityAction, "add_algebraic_rm");

InputParameters
AdaptivityAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Add libMesh based adaptation schemes via the Executioner/Adaptivity input syntax.");
  MooseEnum estimators("KellyErrorEstimator LaplacianErrorEstimator PatchRecoveryErrorEstimator",
                       "KellyErrorEstimator");

  params.addParam<unsigned int>(
      "steps", 0, "The number of adaptivity steps to perform at any one time for steady state");
  params.addRangeCheckedParam<unsigned int>(
      "interval", 1, "interval>0", "The number of time steps betweeen each adaptivity phase");
  params.addParam<unsigned int>(
      "initial_adaptivity",
      0,
      "The number of adaptivity steps to perform using the initial conditions");
  params.addParam<Real>("refine_fraction",
                        0.0,
                        "The fraction of elements or error to refine. Should be between 0 and 1.");
  params.addParam<Real>("coarsen_fraction",
                        0.0,
                        "The fraction of elements or error to coarsen. Should be between 0 and 1.");
  params.addParam<unsigned int>(
      "max_h_level",
      0,
      "Maximum number of times a single element can be refined. If 0 then infinite.");
  params.addParam<MooseEnum>(
      "error_estimator", estimators, "The class name of the error estimator you want to use.");
  params.addDeprecatedParam<bool>(
      "print_changed_info",
      false,
      "Determines whether information about the mesh is printed when adaptivity occurs",
      "Use the Console output parameter 'print_mesh_changed_info'");
  params.addParam<Real>("start_time",
                        -std::numeric_limits<Real>::max(),
                        "The time that adaptivity will be active after.");
  params.addParam<Real>("stop_time",
                        std::numeric_limits<Real>::max(),
                        "The time after which adaptivity will no longer be active.");
  params.addParam<std::vector<std::string>>(
      "weight_names", "List of names of variables that will be associated with weight_values");
  params.addParam<std::vector<Real>>(
      "weight_values",
      "List of values between 0 and 1 to weight the associated weight_names error by");
  params.addParam<unsigned int>("cycles_per_step", 1, "The number of adaptivity cycles per step");

  params.addParam<bool>(
      "show_initial_progress", true, "Show the progress of the initial adaptivity");
  params.addParam<bool>(
      "recompute_markers_during_cycles", false, "Recompute markers during adaptivity cycles");
  return params;
}

AdaptivityAction::AdaptivityAction(const InputParameters & params) : Action(params) {}

void
AdaptivityAction::act()
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

  else if (_current_task == "setup_adaptivity")
  {
    NonlinearSystemBase & system = _problem->getNonlinearSystemBase();

    Adaptivity & adapt = _problem->adaptivity();

    // we don't need to run mesh modifiers *again* after they ran already during the mesh
    // splitting process. Adaptivity::init must be called for any adaptivity to work, however, so we
    // can't just skip it for the useSplit case.
    if (_mesh->isSplit())
      adapt.init(0, 0);
    else
      adapt.init(getParam<unsigned int>("steps"), getParam<unsigned int>("initial_adaptivity"));

    adapt.setErrorEstimator(getParam<MooseEnum>("error_estimator"));

    adapt.setParam("cycles_per_step", getParam<unsigned int>("cycles_per_step"));
    adapt.setParam("refine fraction", getParam<Real>("refine_fraction"));
    adapt.setParam("coarsen fraction", getParam<Real>("coarsen_fraction"));
    adapt.setParam("max h-level", getParam<unsigned int>("max_h_level"));
    adapt.setParam("recompute_markers_during_cycles",
                   getParam<bool>("recompute_markers_during_cycles"));

    adapt.setPrintMeshChanged(getParam<bool>("print_changed_info"));

    const std::vector<std::string> & weight_names =
        getParam<std::vector<std::string>>("weight_names");
    const std::vector<Real> & weight_values = getParam<std::vector<Real>>("weight_values");

    auto num_weight_names = weight_names.size();
    auto num_weight_values = weight_values.size();

    if (num_weight_names)
    {
      if (num_weight_names != num_weight_values)
        mooseError("Number of weight_names must be equal to number of weight_values in "
                   "Execution/Adaptivity");

      // If weights have been specified then set the default weight to zero
      std::vector<Real> weights(system.nVariables(), 0);

      for (MooseIndex(num_weight_names) i = 0; i < num_weight_names; i++)
      {
        std::string name = weight_names[i];
        auto value = weight_values[i];

        weights[system.getVariable(0, name).number()] = value;
      }

      std::vector<FEMNormType> norms(system.nVariables(), H1_SEMINORM);

      SystemNorm sys_norm(norms, weights);

      adapt.setErrorNorm(sys_norm);
    }

    adapt.setTimeActive(getParam<Real>("start_time"), getParam<Real>("stop_time"));
    adapt.setInterval(getParam<unsigned int>("interval"));
  }
}

#endif // LIBMESH_ENABLE_AMR
