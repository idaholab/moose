//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupMeshAction.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "FileMesh.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"
#include "Factory.h"
#include "AddMeshGeneratorAction.h"

#include <functional>
#include <algorithm>

registerMooseAction("MooseApp", SetupMeshAction, "setup_mesh");
registerMooseAction("MooseApp", SetupMeshAction, "set_mesh_base");
registerMooseAction("MooseApp", SetupMeshAction, "init_mesh");

InputParameters
SetupMeshAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add or create Mesh object to the simulation.");

  // Here we are setting the default type of the mesh to construct to "FileMesh". This is to support
  // the very long-running legacy syntax where only a file parameter is required to determine the
  // type of the "Mesh" block. We are re-adding it though so that we can detect whether or not the
  // user has explicitly set the type in an input file. We do this because we want to support
  // automatically building a "MeshGeneratorMesh" type when MeshGenerators are added to the
  // simulation.
  params.addParam<std::string>(
      "type",
      "FileMesh",
      "A string representing the Moose Object that will be built by this Action");

  params.addParam<bool>("second_order",
                        false,
                        "Converts a first order mesh to a second order "
                        "mesh.  Note: This is NOT needed if you are reading "
                        "an actual first order mesh.");

  params.addParam<std::vector<SubdomainID>>("block_id", "IDs of the block id/name pairs");
  params.addParam<std::vector<SubdomainName>>(
      "block_name", "Names of the block id/name pairs (must correspond with \"block_id\"");

  params.addParam<std::vector<BoundaryID>>("boundary_id", "IDs of the boundary id/name pairs");
  params.addParam<std::vector<BoundaryName>>(
      "boundary_name", "Names of the boundary id/name pairs (must correspond with \"boundary_id\"");

  params.addParam<bool>("construct_side_list_from_node_list",
                        false,
                        "If true, construct side lists from the nodesets in the mesh (i.e. if "
                        "every node on a give side is in a nodeset then add that side to a "
                        "sideset");

  params.addParam<std::vector<std::string>>(
      "displacements",
      "The variables corresponding to the x y z displacements of the mesh.  If "
      "this is provided then the displacements will be taken into account during "
      "the computation. Creation of the displaced mesh can be suppressed even if "
      "this is set by setting 'use_displaced_mesh = false'.");
  params.addParam<bool>(
      "use_displaced_mesh",
      true,
      "Create the displaced mesh if the 'displacements' "
      "parameter is set. If this is 'false', a displaced mesh will not be created, "
      "regardless of whether 'displacements' is set.");
  params.addParam<std::vector<BoundaryName>>("ghosted_boundaries",
                                             "Boundaries to be ghosted if using Nemesis");
  params.addParam<std::vector<Real>>("ghosted_boundaries_inflation",
                                     "If you are using ghosted boundaries you will want to set "
                                     "this value to a vector of amounts to inflate the bounding "
                                     "boxes by.  ie if you are running a 3D problem you might set "
                                     "it to '0.2 0.1 0.4'");

  params.addParam<unsigned int>(
      "uniform_refine", 0, "Specify the level of uniform refinement applied to the initial mesh");

  params.addParam<bool>("skip_deletion_repartition_after_refine",
                        false,
                        "If the flag is true, uniform refinements will run more efficiently, "
                        "but at the same time, there might be extra ghosting elements. "
                        "The number of layers of additional ghosting elements depends "
                        "on the number of uniform refinement levels.  This flag "
                        "should be used only when you have a 'fine enough' coarse mesh and want "
                        "to refine the mesh by a few levels. Otherwise, it might introduce an "
                        "unbalanced workload and too large ghosting domain. ");

  params.addParam<bool>("skip_partitioning",
                        false,
                        "If true the mesh won't be partitioned. This may cause large load "
                        "imbalances.");

  params.addParam<bool>(
      "use_split",
      false,
      "Use split distributed mesh files; is overriden by the --use-split command line option");
  params.addParam<std::string>("split_file",
                               "",
                               "Optional name of split mesh file(s) to write/read; is overridden "
                               "by the --split-file command line option");

  // groups
  params.addParamNamesToGroup("displacements ghosted_boundaries ghosted_boundaries_inflation",
                              "Advanced");
  params.addParamNamesToGroup("second_order construct_side_list_from_node_list skip_partitioning",
                              "Advanced");
  params.addParamNamesToGroup("block_id block_name boundary_id boundary_name", "Add Names");
  params.addParamNamesToGroup("use_split split_file", "Split Mesh");

  return params;
}

SetupMeshAction::SetupMeshAction(const InputParameters & params)
  : MooseObjectAction(params),
    _use_split(getParam<bool>("use_split") || _app.getParam<bool>("use_split")),
    _split_file(_app.getParam<std::string>("split_file").size()
                    ? _app.getParam<std::string>("split_file")
                    : getParam<std::string>("split_file"))
{
}

void
SetupMeshAction::setupMesh(MooseMesh * mesh)
{
  std::vector<BoundaryName> ghosted_boundaries =
      getParam<std::vector<BoundaryName>>("ghosted_boundaries");

  for (const auto & bnd_name : ghosted_boundaries)
    mesh->addGhostedBoundary(mesh->getBoundaryID(bnd_name));

  if (isParamValid("ghosted_boundaries_inflation"))
  {
    std::vector<Real> ghosted_boundaries_inflation =
        getParam<std::vector<Real>>("ghosted_boundaries_inflation");
    mesh->setGhostedBoundaryInflation(ghosted_boundaries_inflation);
  }

  mesh->ghostGhostedBoundaries();

  if (getParam<bool>("second_order"))
    mesh->getMesh().all_second_order(true);

#ifdef LIBMESH_ENABLE_AMR
  unsigned int level = getParam<unsigned int>("uniform_refine");

  // Did they specify extra refinement levels on the command-line?
  level += _app.getParam<unsigned int>("refinements");

  mesh->setUniformRefineLevel(level, getParam<bool>("skip_deletion_repartition_after_refine"));
#endif // LIBMESH_ENABLE_AMR

  // Add entity names to the mesh
  if (_pars.isParamValid("block_id") && _pars.isParamValid("block_name"))
  {
    std::vector<SubdomainID> ids = getParam<std::vector<SubdomainID>>("block_id");
    std::vector<SubdomainName> names = getParam<std::vector<SubdomainName>>("block_name");
    std::set<SubdomainName> seen_it;

    if (ids.size() != names.size())
      mooseError("You must supply the same number of block ids and names parameters");

    for (unsigned int i = 0; i < ids.size(); ++i)
    {
      if (seen_it.find(names[i]) != seen_it.end())
        mooseError("The following dynamic block name is not unique: " + names[i]);
      seen_it.insert(names[i]);
      mesh->setSubdomainName(ids[i], names[i]);
    }
  }
  if (_pars.isParamValid("boundary_id") && _pars.isParamValid("boundary_name"))
  {
    std::vector<BoundaryID> ids = getParam<std::vector<BoundaryID>>("boundary_id");
    std::vector<BoundaryName> names = getParam<std::vector<BoundaryName>>("boundary_name");
    std::set<SubdomainName> seen_it;

    if (ids.size() != names.size())
      mooseError("You must supply the same number of boundary ids and names parameters");

    for (unsigned int i = 0; i < ids.size(); ++i)
    {
      if (seen_it.find(names[i]) != seen_it.end())
        mooseError("The following dynamic boundary name is not unique: " + names[i]);
      mesh->setBoundaryName(ids[i], names[i]);
      seen_it.insert(names[i]);
    }
  }

  if (getParam<bool>("construct_side_list_from_node_list"))
    mesh->getMesh().get_boundary_info().build_side_list_from_node_list();

  // Here we can override the partitioning for special cases
  if (getParam<bool>("skip_partitioning"))
    mesh->getMesh().skip_partitioning(getParam<bool>("skip_partitioning"));
}

std::string
SetupMeshAction::modifyParamsForUseSplit(InputParameters & moose_object_params) const
{
  // Get the split_file extension, if there is one, and use that to decide
  // between .cpr and .cpa
  auto split_file = _split_file;
  std::string split_file_ext;
  auto pos = split_file.rfind(".");
  if (pos != std::string::npos)
    split_file_ext = split_file.substr(pos + 1, std::string::npos);

  // If split_file already has the .cpr or .cpa extension, we go with
  // that, otherwise we strip off the extension and append ".cpr".
  if (split_file != "" && split_file_ext != "cpr" && split_file_ext != "cpa")
    split_file = MooseUtils::stripExtension(split_file) + ".cpr";

  if (_type != "FileMesh")
  {
    if (split_file.empty())
      mooseError("Cannot use split mesh for a non-file mesh without specifying --split-file on "
                 "command line or the Mesh/split_file parameter");

    auto new_pars = FileMesh::validParams();

    // Keep existing parameters where possible
    new_pars.applyParameters(_moose_object_pars);

    new_pars.set<MeshFileName>("file") = split_file;
    new_pars.set<MooseApp *>("_moose_app") = moose_object_params.get<MooseApp *>("_moose_app");
    moose_object_params = new_pars;
  }
  else
  {
    if (!split_file.empty())
      moose_object_params.set<MeshFileName>("file") = split_file;
    else
      moose_object_params.set<MeshFileName>("file") =
          MooseUtils::stripExtension(moose_object_params.get<MeshFileName>("file")) + ".cpr";
  }

  moose_object_params.set<bool>("_is_split") = true;

  return "FileMesh";
}

void
SetupMeshAction::act()
{
  // Create the mesh object and tell it to build itself
  if (_current_task == "setup_mesh")
  {
    TIME_SECTION("SetupMeshAction::act::setup_mesh", 1, "Setting Up Mesh", true);

    if (_app.masterMesh())
      _mesh = _app.masterMesh()->safeClone();
    else
    {
      const auto & generator_actions = _awh.getActionListByName("add_mesh_generator");

      // If we trigger any actions that can build MeshGenerators, whether through input file
      // syntax or through custom actions, change the default type to construct. We can't yet
      // check whether there are any actual MeshGenerator objects because those are added after
      // setup_mesh
      if (!generator_actions.empty())
      {
        // Check for whether type has been set or whether for the default type (FileMesh) a file has
        // been provided
        if (!_pars.isParamSetByUser("type") && !_moose_object_pars.isParamValid("file"))
        {
          _type = "MeshGeneratorMesh";
          auto original_params = _moose_object_pars;
          _moose_object_pars = _factory.getValidParams("MeshGeneratorMesh");

          // Since we changing the type on the fly, we'll have to manually extract parameters again
          // from the input file object.
          _app.parser().extractParams(_registered_identifier, _moose_object_pars);
        }
        else if (!_moose_object_pars.get<bool>("_mesh_generator_mesh"))
        {
          // There are cases where a custom action may register the "add_mesh_generator" task, but
          // may not actually add any mesh generators depending on user input. We don't want to risk
          // giving false warnings in this case. However, if we triggered the "add_mesh_generator"
          // task through explicit input file syntax, then it is definitely safe to warn
          for (auto generator_action_ptr : generator_actions)
            if (dynamic_cast<AddMeshGeneratorAction *>(generator_action_ptr))
            {
              mooseWarning("Mesh Generators present but the [Mesh] block is set to construct a \"",
                           _type,
                           "\" mesh, which does not use Mesh Generators in constructing the mesh.");
              break;
            }
        }
      }

      // switch non-file meshes to be a file-mesh if using a pre-split mesh configuration.
      if (_use_split)
        _type = modifyParamsForUseSplit(_moose_object_pars);

      _mesh = _factory.create<MooseMesh>(_type, "mesh", _moose_object_pars);
    }
  }

  else if (_current_task == "set_mesh_base")
  {

    TIME_SECTION("SetupMeshAction::act::set_mesh_base", 1, "Setting Mesh", true);

    if (!_app.masterMesh() && !_mesh->hasMeshBase())
    {
      // We want to set the MeshBase object to that coming from mesh generators when the following
      // conditions are met:
      // 1. We have mesh generators
      // 2. We are not using the pre-split mesh
      // 3. We are not: recovering/restarting and we are the master application
      if (!_app.getMeshGeneratorNames().empty() && !_use_split &&
          !((_app.isRecovering() || _app.isRestarting()) && _app.isUltimateMaster()))
      {
        auto & mesh_generator_system = _app.getMeshGeneratorSystem();
        auto mesh_base =
            mesh_generator_system.getSavedMesh(mesh_generator_system.mainMeshGeneratorName());
        if (_mesh->allowRemoteElementRemoval() != mesh_base->allow_remote_element_removal())
          mooseError("The MooseMesh and libmesh::MeshBase object coming from mesh generators are "
                     "out of sync with respect to whether remote elements can be deleted");
        mooseAssert(mesh_base, "Null mesh");
        _mesh->setMeshBase(std::move(mesh_base));
      }
      else
      {
        const auto & mg_names = _app.getMeshGeneratorNames();
        std::vector<bool> use_dm;
        for (const auto & mg_name : mg_names)
          if (hasMeshProperty<bool>("use_distributed_mesh", mg_name))
            use_dm.push_back(getMeshProperty<bool>("use_distributed_mesh", mg_name));

        if (!use_dm.empty())
        {
          if (std::adjacent_find(use_dm.begin(), use_dm.end(), std::not_equal_to<bool>()) !=
              use_dm.end())
            mooseError("You cannot use mesh generators that set different values of the mesh "
                       "property 'use_distributed_mesh' within the same simulation.");

          const auto ptype = use_dm.front() ? MooseMesh::ParallelType::DISTRIBUTED
                                            : MooseMesh::ParallelType::REPLICATED;
          _mesh->setParallelType(ptype);
        }

        _mesh->setMeshBase(_mesh->buildMeshBaseObject());
      }
    }
  }

  else if (_current_task == "init_mesh")
  {
    TIME_SECTION("SetupMeshAction::act::set_mesh_base", 1, "Initializing Mesh", true);

    if (_app.masterMesh())
    {
      if (_app.masterDisplacedMesh())
        _displaced_mesh = _app.masterDisplacedMesh()->safeClone();
    }
    else
    {
      _mesh->init();

      if (isParamValid("displacements") && getParam<bool>("use_displaced_mesh"))
      {
        _displaced_mesh = _mesh->safeClone();
        _displaced_mesh->isDisplaced(true);
        _displaced_mesh->getMesh().allow_remote_element_removal(
            _mesh->getMesh().allow_remote_element_removal());

        std::vector<std::string> displacements =
            getParam<std::vector<std::string>>("displacements");
        if (displacements.size() < _displaced_mesh->dimension())
          mooseError("Number of displacements must be greater than or equal to the dimension of "
                     "the mesh!");
      }

      setupMesh(_mesh.get());

      if (_displaced_mesh)
        setupMesh(_displaced_mesh.get());
    }
  }
}
