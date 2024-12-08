//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddPeriodicBCAction.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "FunctionPeriodicBoundary.h"
#include "GeneratedMesh.h"
#include "InputParameters.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"
#include "RelationshipManager.h"

#include "libmesh/periodic_boundary.h" // translation PBCs provided by libmesh

using namespace libMesh;

registerMooseAction("MooseApp", AddPeriodicBCAction, "add_periodic_bc");
registerMooseAction("MooseApp", AddPeriodicBCAction, "add_geometric_rm");
registerMooseAction("MooseApp", AddPeriodicBCAction, "add_algebraic_rm");

InputParameters
AddPeriodicBCAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<std::vector<std::string>>("auto_direction",
                                            "If using a generated mesh, you can "
                                            "specify just the dimension(s) you "
                                            "want to mark as periodic");

  params.addParam<BoundaryName>("primary", "Boundary ID associated with the primary boundary.");
  params.addParam<BoundaryName>("secondary", "Boundary ID associated with the secondary boundary.");
  params.addParam<RealVectorValue>("translation",
                                   "Vector that translates coordinates on the "
                                   "primary boundary to coordinates on the "
                                   "secondary boundary.");
  params.addParam<std::vector<std::string>>("transform_func",
                                            "Functions that specify the transformation");
  params.addParam<std::vector<std::string>>("inv_transform_func",
                                            "Functions that specify the inverse transformation");

  params.addParam<std::vector<VariableName>>(
      "variable", {}, "Variable for the periodic boundary condition");
  params.addClassDescription("Action that adds periodic boundary conditions");
  return params;
}

AddPeriodicBCAction::AddPeriodicBCAction(const InputParameters & params)
  : Action(params), _mesh(nullptr)
{
  // Check for inconsistent parameters
  if (isParamValid("auto_direction"))
  {
    if (isParamValid("primary") || isParamValid("secondary") || isParamValid("translation") ||
        isParamValid("transform_func") || isParamValid("inv_transform_func"))
      paramError(
          "auto_direction",
          "Using the automatic periodic boundary detection does not require additional parameters");
  }
  else if (!isParamValid("primary") || !isParamValid("secondary"))
    paramError("primary", "Both a primary and secondary boundary must be specified");
}

void
AddPeriodicBCAction::setPeriodicVars(libMesh::PeriodicBoundaryBase & p,
                                     const std::vector<VariableName> & var_names)
{
  // TODO: multi-system
  if (_problem->numSolverSystems() > 1)
    mooseError("Multiple solver systems currently not supported");

  NonlinearSystemBase & nl = _problem->getNonlinearSystemBase(/*nl_sys_num=*/0);
  const std::vector<VariableName> * var_names_ptr;

  // If var_names is empty - then apply this periodic condition to all variables in the system
  if (var_names.empty())
    var_names_ptr = &nl.getVariableNames();
  else
    var_names_ptr = &var_names;

  for (const auto & var_name : *var_names_ptr)
  {
    // Exclude scalar variables for which periodic boundary conditions dont make sense
    if (!nl.hasScalarVariable(var_name))
    {
      unsigned int var_num = nl.getVariable(0, var_name).number();
      p.set_variable(var_num);
      // Only try to add periodic information to meshes which currently support them
      if (_mesh->isRegularOrthogonal())
        _mesh->addPeriodicVariable(var_num, p.myboundary, p.pairedboundary);
      else
        mooseInfoRepeated("Periodicity information for variable '" + var_name +
                          "' will only be stored in the system's DoF map, not on the MooseMesh");
    }
  }
}

bool
AddPeriodicBCAction::autoTranslationBoundaries()
{
  auto displaced_problem = _problem->getDisplacedProblem();

  if (isParamValid("auto_direction"))
  {
    // If we are working with a parallel mesh then we're going to ghost all the boundaries
    // everywhere because we don't know what we need...
    if (_mesh->isDistributedMesh())
    {
      bool is_orthogonal_mesh = _mesh->detectOrthogonalDimRanges();

      // If we can't detect the orthogonal dimension ranges for this
      // Mesh, then auto_direction periodicity isn't going to work.
      if (!is_orthogonal_mesh)
        mooseError("Could not detect orthogonal dimension ranges for DistributedMesh.");
    }

    std::vector<std::string> auto_dirs = getParam<std::vector<std::string>>("auto_direction");

    int dim_offset = _mesh->dimension() - 2;
    for (const auto & dir : auto_dirs)
    {
      int component = -1;
      if (dir == "X" || dir == "x")
        component = 0;
      else if (dir == "Y" || dir == "y")
      {
        if (dim_offset < 0)
          mooseError("Cannot wrap 'Y' direction when using a 1D mesh");
        component = 1;
      }
      else if (dir == "Z" || dir == "z")
      {
        if (dim_offset <= 0)
          mooseError("Cannot wrap 'Z' direction when using a 1D or 2D mesh");
        component = 2;
      }

      if (component >= 0)
      {
        const std::pair<BoundaryID, BoundaryID> * boundary_ids =
            _mesh->getPairedBoundaryMapping(component);
        RealVectorValue v;
        v(component) = _mesh->dimensionWidth(component);
        PeriodicBoundary p(v);

        if (boundary_ids == nullptr)
          mooseError("Couldn't auto-detect a paired boundary for use with periodic boundary "
                     "conditions in the '" +
                     dir + "' direction");

        p.myboundary = boundary_ids->first;
        p.pairedboundary = boundary_ids->second;
        setPeriodicVars(p, getParam<std::vector<VariableName>>("variable"));
        auto & eq = _problem->es();
        for (const auto i : make_range(eq.n_systems()))
          eq.get_system(i).get_dof_map().add_periodic_boundary(p);
        if (displaced_problem)
        {
          auto & deq = displaced_problem->es();
          for (const auto i : make_range(deq.n_systems()))
            deq.get_system(i).get_dof_map().add_periodic_boundary(p);
        }
      }
    }
    return true;
  }
  return false;
}

void
AddPeriodicBCAction::act()
{
  if (_current_task == "add_geometric_rm")
    // Tell the mesh to hold off on deleting remote elements because we need to wait for our
    // periodic boundaries to be added
    Action::_mesh->allowRemoteElementRemoval(false);

  if (_current_task == "add_algebraic_rm")
  {
    auto rm_params = _factory.getValidParams("ElementSideNeighborLayers");

    rm_params.set<std::string>("for_whom") = "PeriodicBCs";
    if (!_mesh)
      mooseError("We should have added periodic boundaries and consequently we should have set the "
                 "_mesh by now");

    rm_params.set<MooseMesh *>("mesh") = _mesh;
    // The default GhostPointNeighbors ghosting functor in libMesh handles the geometric ghosting
    // of periodic boundaries for us, so we only need to handle the algebraic ghosting here
    rm_params.set<Moose::RelationshipManagerType>("rm_type") =
        Moose::RelationshipManagerType::ALGEBRAIC;

    if (rm_params.areAllRequiredParamsValid())
    {
      auto rm_obj = _factory.create<RelationshipManager>(
          "ElementSideNeighborLayers", "periodic_bc_ghosting_" + name(), rm_params);

      if (!_app.addRelationshipManager(rm_obj))
        _factory.releaseSharedObjects(*rm_obj);
    }
    else
      mooseError("Invalid initialization of ElementSideNeighborLayers");
  }

  if (_current_task == "add_periodic_bc")
  {
    auto & nl = _problem->getNonlinearSystemBase(/*nl_sys_num=*/0);
    _mesh = &_problem->mesh();
    auto displaced_problem = _problem->getDisplacedProblem();

    if (!autoTranslationBoundaries())
    {
      // Check that the boundaries exist in the mesh
      const auto & primary_name = getParam<BoundaryName>("primary");
      const auto & secondary_name = getParam<BoundaryName>("secondary");
      if (!MooseMeshUtils::hasBoundaryName(*_mesh, primary_name))
        paramError("primary", "Boundary '" + primary_name + "' does not exist in the mesh");
      if (!MooseMeshUtils::hasBoundaryName(*_mesh, secondary_name))
        paramError("secondary", "Boundary '" + secondary_name + "' does not exist in the mesh");

      if (_pars.isParamValid("translation"))
      {
        RealVectorValue translation = getParam<RealVectorValue>("translation");

        PeriodicBoundary p(translation);
        p.myboundary = _mesh->getBoundaryID(primary_name);
        p.pairedboundary = _mesh->getBoundaryID(secondary_name);
        setPeriodicVars(p, getParam<std::vector<VariableName>>("variable"));

        auto & eq = _problem->es();
        for (const auto i : make_range(eq.n_systems()))
          eq.get_system(i).get_dof_map().add_periodic_boundary(p);
        if (displaced_problem)
        {
          auto & deq = displaced_problem->es();
          for (const auto i : make_range(deq.n_systems()))
            deq.get_system(i).get_dof_map().add_periodic_boundary(p);
        }
      }
      else if (getParam<std::vector<std::string>>("transform_func") != std::vector<std::string>())
      {
        std::vector<std::string> inv_fn_names =
            getParam<std::vector<std::string>>("inv_transform_func");
        std::vector<std::string> fn_names = getParam<std::vector<std::string>>("transform_func");

        // If the user provided a forward transformation, they must also provide an inverse -- we
        // can't form the inverse of an arbitrary function automatically...
        if (inv_fn_names == std::vector<std::string>())
          mooseError("You must provide an inv_transform_func for FunctionPeriodicBoundary!");

        FunctionPeriodicBoundary pb(*_problem, fn_names);
        pb.myboundary = _mesh->getBoundaryID(primary_name);
        pb.pairedboundary = _mesh->getBoundaryID(secondary_name);
        setPeriodicVars(pb, getParam<std::vector<VariableName>>("variable"));

        FunctionPeriodicBoundary ipb(*_problem, inv_fn_names);
        ipb.myboundary = _mesh->getBoundaryID(secondary_name);   // these are swapped
        ipb.pairedboundary = _mesh->getBoundaryID(primary_name); // these are swapped
        setPeriodicVars(ipb, getParam<std::vector<VariableName>>("variable"));

        // Add the pair of periodic boundaries to the dof map
        auto & eq = _problem->es();
        for (const auto i : make_range(eq.n_systems()))
          eq.get_system(i).get_dof_map().add_periodic_boundary(pb, ipb);
        if (displaced_problem)
        {
          auto & deq = displaced_problem->es();
          for (const auto i : make_range(deq.n_systems()))
            deq.get_system(i).get_dof_map().add_periodic_boundary(pb, ipb);
        }
      }
      else
      {
        mooseError(
            "You have to specify either 'auto_direction', 'translation' or 'trans_func' in your "
            "period boundary section '" +
            _name + "'");
      }
    }

    // Now make sure that the mesh default ghosting functor has its periodic bcs set
    // TODO: multi-system
    _mesh->getMesh().default_ghosting().set_periodic_boundaries(
        nl.dofMap().get_periodic_boundaries());
    if (displaced_problem)
      displaced_problem->mesh().getMesh().default_ghosting().set_periodic_boundaries(
          displaced_problem->solverSys(/*nl_sys_num=*/0).dofMap().get_periodic_boundaries());
  }
}
