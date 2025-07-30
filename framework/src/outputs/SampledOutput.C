//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SampledOutput.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MooseApp.h"
#include "MoosePartitioner.h"

#include "libmesh/distributed_mesh.h"
#include "libmesh/equation_systems.h"
#include "libmesh/mesh_function.h"
#include "libmesh/explicit_system.h"

using namespace libMesh;

InputParameters
SampledOutput::validParams()
{

  // Get the parameters from the parent object
  InputParameters params = AdvancedOutput::validParams();
  params.addParam<unsigned int>("refinements",
                                0,
                                "Number of uniform refinements for oversampling "
                                "(refinement levels beyond any level of "
                                "refinements already applied on the regular mesh)");
  params.addParam<Point>("position",
                         "Set a positional offset, this vector will get added to the "
                         "nodal coordinates to move the domain.");
  params.addParam<MeshFileName>("file", "The name of the mesh file to read, for oversampling");
  params.addParam<std::vector<SubdomainName>>(
      "sampling_blocks", "The list of blocks to restrict the mesh sampling to");
  params.addParam<bool>(
      "serialize_sampling",
      true,
      "If set to true, all sampled output (see sampling parameters) will be done "
      "on rank 0. This option is useful to debug suspected parallel output issues");

  // **** DEPRECATED PARAMETERS ****
  params.addDeprecatedParam<bool>("append_oversample",
                                  false,
                                  "Append '_oversample' to the output file base",
                                  "This parameter is deprecated. To append '_oversample' utilize "
                                  "the output block name or the 'file_base'");

  // 'Oversampling' Group
  params.addParamNamesToGroup("refinements position file sampling_blocks serialize_sampling",
                              "Modified Mesh Sampling");

  return params;
}

SampledOutput::SampledOutput(const InputParameters & parameters)
  : AdvancedOutput(parameters),
    _refinements(getParam<unsigned int>("refinements")),
    _using_external_sampling_file(isParamValid("file")),
    _change_position(isParamValid("position")),
    _use_sampled_output(_refinements > 0 || _using_external_sampling_file ||
                        isParamValid("sampling_blocks") || _change_position),
    _position(_change_position ? getParam<Point>("position") : Point()),
    _sampling_mesh_changed(true),
    _mesh_subdomains_match(true),
    _serialize(getParam<bool>("serialize_sampling"))
{
}

void
SampledOutput::initialSetup()
{
  AdvancedOutput::initialSetup();

  // Creates and initializes the sampling mesh
  initSample();
}

void
SampledOutput::outputStep(const ExecFlagType & type)
{
  // Output is not allowed
  if (!_allow_output && type != EXEC_FORCED)
    return;

  // If recovering disable output of initial condition, it was already output
  if (type == EXEC_INITIAL && _app.isRecovering())
    return;

  // Return if the current output is not on the desired interval
  if (type != EXEC_FINAL && !onInterval())
    return;

  // store current simulation time
  _last_output_simulation_time = _time;

  // set current type
  _current_execute_flag = type;

  // Call the output method
  if (shouldOutput())
  {
    TIME_SECTION("outputStep", 2, "Outputting Step");
    updateSample();
    output();
  }

  _current_execute_flag = EXEC_NONE;
}

SampledOutput::~SampledOutput()
{
  // TODO: Remove once libmesh Issue #1184 is fixed
  _sampling_es.reset();
  _sampling_mesh_ptr.reset();
}

void
SampledOutput::meshChanged()
{
  _sampling_mesh_changed = true;
}

void
SampledOutput::initSample()
{
  // Perform the mesh cloning, if needed
  if (!_use_sampled_output)
    return;

  cloneMesh();

  // Re-position the sampling mesh
  if (_change_position)
    for (auto & node : _mesh_ptr->getMesh().node_ptr_range())
      *node += _position;

  // Perform the mesh refinement
  if (_refinements > 0)
  {
    MeshRefinement mesh_refinement(_mesh_ptr->getMesh());

    // We want original and refined partitioning to match so we can
    // query from one to the other safely on distributed meshes.
    _mesh_ptr->getMesh().skip_partitioning(true);
    mesh_refinement.uniformly_refine(_refinements);

    // Note that nodesets are not propagated with mesh refinement, unless you built the nodesets
    // from the sidesets again, which is what happens for the regular mesh with initial refinement
  }

  // We can't allow renumbering if we want to output multiple time
  // steps to the same Exodus file
  _mesh_ptr->getMesh().allow_renumbering(false);

  // This should be called after changing the mesh (block restriction for example)
  if (_change_position || (_refinements > 0) || isParamValid("sampling_blocks"))
    _sampling_mesh_ptr->meshChanged();

  // Create the new EquationSystems
  _sampling_es = std::make_unique<EquationSystems>(_mesh_ptr->getMesh());
  _es_ptr = _sampling_es.get();

  // Reference the system from which we are copying
  EquationSystems & source_es = _problem_ptr->es();

  // If we're going to be copying from that system later, we need to keep its
  // original elements as ghost elements even if it gets grossly
  // repartitioned, since we can't repartition the sample mesh to
  // match.
  // FIXME: this is not enough. It assumes our initial partition of the sampling mesh
  // and the source mesh match. But that's usually only true in the 'refinement' case,
  // not with an arbitrary sampling mesh file
  DistributedMesh * dist_mesh = dynamic_cast<DistributedMesh *>(&source_es.get_mesh());
  if (dist_mesh)
  {
    for (auto & elem : dist_mesh->active_local_element_ptr_range())
      dist_mesh->add_extra_ghost_elem(elem);
  }

  // Initialize the _mesh_functions vector
  const auto num_systems = source_es.n_systems();
  _mesh_functions.resize(num_systems);

  // Keep track of the variable numbering in both regular and sampled system
  _variable_numbers_in_system.resize(num_systems);

  // Get the list of nodal and elemental output data
  const auto & nodal_data = getNodalVariableOutput();
  const auto & elemental_data = getElementalVariableOutput();

  // Loop over the number of systems
  for (const auto sys_num : make_range(num_systems))
  {
    // Reference to the current system
    const auto & source_sys = source_es.get_system(sys_num);

    // Add the system to the new EquationsSystems
    ExplicitSystem & dest_sys = _sampling_es->add_system<ExplicitSystem>(source_sys.name());

    // Loop through the variables in the System
    const auto num_vars = source_sys.n_vars();
    unsigned int num_actual_vars = 0;
    if (num_vars > 0)
    {
      if (_serialize)
        _serialized_solution = NumericVector<Number>::build(_communicator);

      // Add the variables to the system... simultaneously creating MeshFunctions for them.
      for (const auto var_num : make_range(num_vars))
      {
        // Is the variable supposed to be output?
        const auto & var_name = source_sys.variable_name(var_num);
        if (!nodal_data.count(var_name) && !elemental_data.count(var_name))
          continue;

        // We do what we can to preserve the block restriction
        const std::set<SubdomainID> * subdomains;
        std::set<SubdomainID> restricted_subdomains;
        if (_using_external_sampling_file && !_mesh_subdomains_match)
          subdomains = nullptr;
        else
        {
          subdomains = &source_sys.variable(var_num).active_subdomains();
          // Reduce the block restriction if the output is block restricted
          if (isParamValid("sampling_blocks") && !subdomains->empty())
          {
            const auto & sampling_blocks = _sampling_mesh_ptr->getSubdomainIDs(
                getParam<std::vector<SubdomainName>>("sampling_blocks"));
            set_intersection(subdomains->begin(),
                             subdomains->end(),
                             sampling_blocks.begin(),
                             sampling_blocks.end(),
                             std::inserter(restricted_subdomains, restricted_subdomains.begin()));
            subdomains = &restricted_subdomains;

            // None of the subdomains are included in the sampling, might as well skip
            if (subdomains->empty())
            {
              hideAdditionalVariable(nodal_data.count(var_name) ? "nodal" : "elemental", var_name);
              continue;
            }
          }
        }

        // We are going to add the variable, let's count it
        _variable_numbers_in_system[sys_num].push_back(var_num);
        num_actual_vars++;

        // Add the variable. We essentially support nodal variables and constant monomials
        const FEType & fe_type = source_sys.variable_type(var_num);
        if (isSampledAtNodes(fe_type))
        {
          dest_sys.add_variable(source_sys.variable_name(var_num), fe_type, subdomains);
          if (dist_mesh && !_serialize)
            paramError("serialize_sampling",
                       "Variables sampled as nodal currently require serialization with a "
                       "distributed mesh.");
        }
        else
        {
          const auto & var_name = source_sys.variable_name(var_num);
          if (fe_type != FEType(CONSTANT, MONOMIAL))
          {
            mooseInfoRepeated("Sampled output projects variable '" + var_name +
                              "' onto a constant monomial");
            if (!_serialize)
              paramWarning("serialize_sampling",
                           "Projection without serialization may fail with insufficient ghosting. "
                           "Consider setting 'serialize_sampling' to true.");
          }
          dest_sys.add_variable(var_name, FEType(CONSTANT, MONOMIAL), subdomains);
        }
        // Note: we could do more, using the generic projector. But exodus output of higher order
        // or more exotic variables is limited anyway
      }

      // Size for the actual number of variables output
      _mesh_functions[sys_num].resize(num_actual_vars);
    }
  }

  // Initialize the newly created EquationSystem
  _sampling_es->init();
}

void
SampledOutput::updateSample()
{
  // Do nothing if oversampling and changing position are not enabled
  if (!_use_sampled_output)
    return;

  // We need the mesh functions to extend the whole domain so we serialize both the mesh and the
  // solution. We need this because the partitioning of the sampling mesh may not match the
  // partitioning of the source mesh
  if (_serialize)
  {
    _problem_ptr->mesh().getMesh().gather_to_zero();
    _mesh_ptr->getMesh().gather_to_zero();
  }

  // Get a reference to actual equation system
  EquationSystems & source_es = _problem_ptr->es();
  const auto num_systems = source_es.n_systems();

  // Loop through each system
  for (const auto sys_num : make_range(num_systems))
  {
    if (!_mesh_functions[sys_num].empty())
    {
      // Get references to the source and destination systems
      System & source_sys = source_es.get_system(sys_num);
      System & dest_sys = _sampling_es->get_system(sys_num);

      // Update the solution for the sampling mesh
      if (_serialize)
      {
        _serialized_solution->clear();
        _serialized_solution->init(source_sys.n_dofs(), false, SERIAL);
        // Pull down a full copy of this vector on every processor so we can get values in
        // parallel
        source_sys.solution->localize(*_serialized_solution);
      }

      // Update the mesh functions
      for (const auto var_num : index_range(_mesh_functions[sys_num]))
      {
        const auto original_var_num = _variable_numbers_in_system[sys_num][var_num];

        // If the mesh has changed, the MeshFunctions need to be re-built, otherwise simply clear
        // it for re-initialization
        // TODO: inherit from MeshChangedInterface and rebuild mesh functions on meshChanged()
        if (!_mesh_functions[sys_num][var_num] || _sampling_mesh_changed)
          _mesh_functions[sys_num][var_num] = std::make_unique<MeshFunction>(
              source_es,
              _serialize ? *_serialized_solution : *source_sys.solution,
              source_sys.get_dof_map(),
              original_var_num);
        else
          _mesh_functions[sys_num][var_num]->clear();

        // Initialize the MeshFunctions for application to the sampled solution
        _mesh_functions[sys_num][var_num]->init();

        // Mesh functions are still defined on the original mesh, which might not fully overlap
        // with the sampling mesh. We don't want to error with a libMesh assert on the out of mesh
        // mode
        _mesh_functions[sys_num][var_num]->enable_out_of_mesh_mode(-1e6);
      }

      // Fill solution vectors by evaluating mesh functions on sampling mesh
      for (const auto var_num : index_range(_mesh_functions[sys_num]))
      {
        // we serialized the mesh and the solution vector, we might as well just do this only on
        // processor 0.
        if (_serialize && processor_id() > 0)
          break;

        const auto original_var_num = _variable_numbers_in_system[sys_num][var_num];
        const FEType & fe_type = source_sys.variable_type(original_var_num);
        // we use the original variable block restriction for sampling
        const auto * var_blocks = &source_sys.variable(original_var_num).active_subdomains();

        // Loop over the mesh, nodes for nodal data, elements for element data
        if (isSampledAtNodes(fe_type))
        {
          for (const auto & node : (_serialize ? _mesh_ptr->getMesh().node_ptr_range()
                                               : _mesh_ptr->getMesh().local_node_ptr_range()))
          {
            // Avoid working on ghosted dofs
            if (node->n_dofs(sys_num, var_num) &&
                (_serialize || processor_id() == node->processor_id()))
            {
              // the node has to be within the domain of the mesh function
              DenseVector<Real> value;
              (*_mesh_functions[sys_num][var_num])(
                  *node - _position, /*time*/ 0., value, var_blocks);

              if (value[0] != -1e6)
                dest_sys.solution->set(node->dof_number(sys_num, var_num, /*comp=*/0), value[0]);
              else
                mooseDoOnce(mooseWarning(
                    "Sampling at location ",
                    *node - _position,
                    " by process ",
                    std::to_string(processor_id()),
                    " was outside the problem mesh.\nThis message will not be repeated"));
            }
          }
        }
        else
        {
          const auto elem_range = _serialize
                                      ? _mesh_ptr->getMesh().active_element_ptr_range()
                                      : _mesh_ptr->getMesh().active_local_element_ptr_range();
          for (const auto & elem : elem_range)
          {
            if (elem->n_dofs(sys_num, var_num) &&
                (_serialize || processor_id() == elem->processor_id()))
            {
              DenseVector<Real> value;
              (*_mesh_functions[sys_num][var_num])(
                  elem->true_centroid() - _position, /*time*/ 0., value, var_blocks);

              if (value[0] != -1e6)
                dest_sys.solution->set(elem->dof_number(sys_num, var_num, /*comp=*/0), value[0]);
              else
                mooseDoOnce(mooseWarning(
                    "Sampling at location ",
                    elem->true_centroid() - _position,
                    " was outside the problem mesh.\nThis message will not be repeated."));
            }
          }
        }
      }

      // We modified the solution vector directly, we have to close it
      dest_sys.solution->close();
    }
  }

  // Set this to false so that new output files are not created, since the sampling mesh
  // doesn't actually change
  _sampling_mesh_changed = false;
}

void
SampledOutput::cloneMesh()
{
  // Create the new mesh from a file
  if (isParamValid("file"))
  {
    InputParameters mesh_params = _app.getFactory().getValidParams("FileMesh");
    mesh_params.applyParameters(parameters(), {}, true);
    mesh_params.set<bool>("nemesis") = false;
    _sampling_mesh_ptr =
        _app.getFactory().createUnique<MooseMesh>("FileMesh", "output_problem_mesh", mesh_params);
    _sampling_mesh_ptr->allowRecovery(false); // We actually want to reread the initial mesh
    _sampling_mesh_ptr->init();
  }
  // Clone the existing mesh
  else
  {
    if (_app.isRecovering())
      mooseWarning("Recovering or Restarting with oversampling may not work (especially with "
                   "adapted meshes)!!  Refs #2295");
    _sampling_mesh_ptr = _mesh_ptr->safeClone();
  }

  // Remove unspecified blocks
  if (isParamValid("sampling_blocks"))
  {
    // Remove all elements not in the blocks
    const auto & blocks_to_keep_names = getParam<std::vector<SubdomainName>>("sampling_blocks");
    const auto & blocks_to_keep = _sampling_mesh_ptr->getSubdomainIDs(blocks_to_keep_names);
    for (const auto & elem_ptr : _sampling_mesh_ptr->getMesh().element_ptr_range())
      if (std::find(blocks_to_keep.begin(), blocks_to_keep.end(), elem_ptr->subdomain_id()) ==
          blocks_to_keep.end())
        _sampling_mesh_ptr->getMesh().delete_elem(elem_ptr);

    // Deleting elements and isolated nodes would cause renumbering. Not renumbering might help
    // user examining the sampling mesh and the regular mesh. Also if we end up partitioning the
    // elements, the node partitioning is unlikely to match if the element numbering is different.
    // Still not enough of a guarantee, because of deleted elements the node partitioning could be
    // different. We will rely on ghosting to make it work
    _sampling_mesh_ptr->getMesh().allow_renumbering(false);
  }

  // Set a partitioner
  if (!_serialize)
  {
    _sampling_mesh_ptr->setIsCustomPartitionerRequested(true);
    InputParameters partition_params = _app.getFactory().getValidParams("CopyMeshPartitioner");
    partition_params.set<MooseMesh *>("mesh") = _sampling_mesh_ptr.get();
    partition_params.set<MooseMesh *>("source_mesh") = _mesh_ptr;
    std::shared_ptr<MoosePartitioner> mp = _factory.create<MoosePartitioner>(
        "CopyMeshPartitioner", "sampled_output_part", partition_params);
    _sampling_mesh_ptr->setCustomPartitioner(mp.get());

    _sampling_mesh_ptr->getMesh().prepare_for_use();
    // this should be called by prepare_for_use, but is not.
    // it also requires a prior call to prepare_for_use()
    mp->partition(_sampling_mesh_ptr->getMesh(), comm().size());
  }

  // Prepare mesh, needed for the mesh functions
  if (_using_external_sampling_file)
    _sampling_mesh_ptr->prepare(/*mesh to clone*/ nullptr);
  else if (_serialize && isParamValid("sampling_blocks"))
    // TODO: constraints have not been initialized?
    _sampling_mesh_ptr->getMesh().prepare_for_use();

  if (_serialize)
    // we want to avoid re-partitioning, as we will serialize anyway
    _sampling_mesh_ptr->getMesh().skip_partitioning(true);

  // Make sure that the mesh pointer points to the newly cloned mesh
  _mesh_ptr = _sampling_mesh_ptr.get();

  // Check the source and target mesh in case their subdomains match
  const std::vector<SubdomainID> mesh_subdomain_ids_vec(_mesh_ptr->meshSubdomains().begin(),
                                                        _mesh_ptr->meshSubdomains().end());
  const std::vector<SubdomainID> initial_mesh_subdomain_ids_vec(
      _problem_ptr->mesh().meshSubdomains().begin(), _problem_ptr->mesh().meshSubdomains().end());
  _mesh_subdomains_match =
      (_mesh_ptr->meshSubdomains() == _problem_ptr->mesh().meshSubdomains() &&
       _mesh_ptr->getSubdomainNames(mesh_subdomain_ids_vec) ==
           _problem_ptr->mesh().getSubdomainNames(initial_mesh_subdomain_ids_vec));
  if (_using_external_sampling_file && !_mesh_subdomains_match)
    mooseInfoRepeated("Variable block restriction disabled in sampled output due to non-matching "
                      "subdomain names and ids");
}

void
SampledOutput::setFileBaseInternal(const std::string & file_base)
{
  AdvancedOutput::setFileBaseInternal(file_base);
  // ** DEPRECATED SUPPORT **
  if (getParam<bool>("append_oversample"))
    _file_base += "_oversample";
}

bool
SampledOutput::isSampledAtNodes(const FEType & fe_type) const
{
  // This is the same criterion as in MooseVariableData
  const auto continuity = FEInterface::get_continuity(fe_type);
  return (continuity == C_ZERO || continuity == C_ONE);
}
