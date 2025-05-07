//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "OversampleOutput.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MooseApp.h"

#include "libmesh/distributed_mesh.h"
#include "libmesh/equation_systems.h"
#include "libmesh/mesh_function.h"
#include "libmesh/explicit_system.h"

using namespace libMesh;

InputParameters
OversampleOutput::validParams()
{

  // Get the parameters from the parent object
  InputParameters params = AdvancedOutput::validParams();
  params.addParam<unsigned int>("refinements",
                                0,
                                "Number of uniform refinements for oversampling "
                                "(refinement levels beyond any uniform "
                                "refinements)");
  params.addParam<Point>("position",
                         "Set a positional offset, this vector will get added to the "
                         "nodal coordinates to move the domain.");
  params.addParam<MeshFileName>("file", "The name of the mesh file to read, for oversampling");
  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of blocks to restrict the mesh sampling to");

  // **** DEPRECATED PARAMETERS ****
  params.addDeprecatedParam<bool>("append_oversample",
                                  false,
                                  "Append '_oversample' to the output file base",
                                  "This parameter is no longer operational, to append "
                                  "'_oversample' utilize the output block name or 'file_base'");

  // 'Oversampling' Group
  params.addParamNamesToGroup("refinements position file block", "Modified Mesh Sampling");

  return params;
}

OversampleOutput::OversampleOutput(const InputParameters & parameters)
  : AdvancedOutput(parameters),
    _refinements(getParam<unsigned int>("refinements")),
    _oversample(_refinements > 0 || isParamValid("file") || isParamValid("block")),
    _change_position(isParamValid("position")),
    _position(_change_position ? getParam<Point>("position") : Point()),
    _oversample_mesh_changed(true)
{
}

void
OversampleOutput::initialSetup()
{
  AdvancedOutput::initialSetup();

  // Creates and initializes the oversampled mesh
  initOversample();
}

void
OversampleOutput::outputStep(const ExecFlagType & type)
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
    updateOversample();
    output();
  }

  _current_execute_flag = EXEC_NONE;
}

OversampleOutput::~OversampleOutput()
{
  // TODO: Remove once libmesh Issue #1184 is fixed
  _oversample_es.reset();
  _cloned_mesh_ptr.reset();
}

void
OversampleOutput::meshChanged()
{
  _oversample_mesh_changed = true;
}

void
OversampleOutput::initOversample()
{
  // Perform the mesh cloning, if needed
  if (_change_position || _oversample)
    cloneMesh();
  else
    return;

  // Re-position the oversampled mesh
  if (_change_position)
    for (auto & node : _mesh_ptr->getMesh().node_ptr_range())
      *node += _position;

  // Perform the mesh refinement
  if (_oversample)
  {
    MeshRefinement mesh_refinement(_mesh_ptr->getMesh());

    // We want original and refined partitioning to match so we can
    // query from one to the other safely on distributed meshes.
    _mesh_ptr->getMesh().skip_partitioning(true);
    mesh_refinement.uniformly_refine(_refinements);
  }

  // We can't allow renumbering if we want to output multiple time
  // steps to the same Exodus file
  _mesh_ptr->getMesh().allow_renumbering(false);

  // Create the new EquationSystems
  _oversample_es = std::make_unique<EquationSystems>(_mesh_ptr->getMesh());
  _es_ptr = _oversample_es.get();

  // Reference the system from which we are copying
  EquationSystems & source_es = _problem_ptr->es();

  // If we're going to be copying from that system later, we need to keep its
  // original elements as ghost elements even if it gets grossly
  // repartitioned, since we can't repartition the oversample mesh to
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
  unsigned int num_systems = source_es.n_systems();
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
    System & source_sys = source_es.get_system(sys_num);

    // Add the system to the new EquationsSystems
    ExplicitSystem & dest_sys = _oversample_es->add_system<ExplicitSystem>(source_sys.name());

    // Loop through the variables in the System
    const auto num_vars = source_sys.n_vars();
    unsigned int num_actual_vars = 0;
    if (num_vars > 0)
    {
      _serialized_solution = NumericVector<Number>::build(_communicator);
      _serialized_solution->init(source_sys.n_dofs(), false, SERIAL);

      // Need to pull down a full copy of this vector on every processor so we can get values in
      // parallel
      source_sys.solution->localize(*_serialized_solution);

      // Add the variables to the system... simultaneously creating MeshFunctions for them.
      for (const auto var_num : make_range(num_vars))
      {
        // Is the variable supposed to be output?
        const auto & var_name = source_sys.variable_name(var_num);
        if (!nodal_data.count(var_name) && !elemental_data.count(var_name))
          continue;
        _variable_numbers_in_system[sys_num].push_back(var_num);
        num_actual_vars++;

        // Add the variable. We essentially support nodal variables and constant monomials
        const FEType & fe_type = source_sys.variable_type(var_num);
        if (isOversampledAsNodal(fe_type))
          dest_sys.add_variable(source_sys.variable_name(var_num), fe_type);
        else
          dest_sys.add_variable(source_sys.variable_name(var_num), FEType(CONSTANT, MONOMIAL));
        // Note: we could do more, using the generic projector. But exodus output of higher order
        // or more exotic variables is limited anyway
      }

      // Size for the actual number of variables output
      _mesh_functions[sys_num].resize(num_actual_vars);
    }
  }

  // Initialize the newly created EquationSystem
  _oversample_es->init();
}

void
OversampleOutput::updateOversample()
{
  // Do nothing if oversampling and changing position are not enabled
  if (!_oversample && !_change_position)
    return;

  // We need the mesh functions to extend the whole domain so we serialize both the mesh and the
  // solution. We need this because the partitioning of the sampling mesh may not match the
  // partitioning of the source mesh
  _problem_ptr->mesh().getMesh().gather_to_zero();

  // Get a reference to actual equation system
  EquationSystems & source_es = _problem_ptr->es();
  unsigned int num_systems = source_es.n_systems();

  // Loop through each system
  for (const auto sys_num : make_range(num_systems))
  {
    if (!_mesh_functions[sys_num].empty())
    {
      // Get references to the source and destination systems
      System & source_sys = source_es.get_system(sys_num);
      System & dest_sys = _oversample_es->get_system(sys_num);

      // Update the solution for the oversampled mesh
      _serialized_solution->clear();
      _serialized_solution->init(source_sys.n_dofs(), false, SERIAL);
      source_sys.solution->localize(*_serialized_solution);

      // Update the mesh functions
      for (const auto var_num : index_range(_mesh_functions[sys_num]))
      {
        const auto original_var_num = _variable_numbers_in_system[sys_num][var_num];
        // If the mesh has changed, the MeshFunctions need to be re-built, otherwise simply clear it
        // for re-initialization
        if (!_mesh_functions[sys_num][var_num] || _oversample_mesh_changed)
          _mesh_functions[sys_num][var_num] = std::make_unique<MeshFunction>(
              source_es, *_serialized_solution, source_sys.get_dof_map(), original_var_num);
        else
          _mesh_functions[sys_num][var_num]->clear();

        // Initialize the MeshFunctions for application to the oversampled solution
        _mesh_functions[sys_num][var_num]->init();

        // Mesh functions are still defined on the original mesh, which might not fully overlap with
        // the sampling mesh. We don't want to error with a libMesh assert on the out of mesh mode
        _mesh_functions[sys_num][var_num]->enable_out_of_mesh_mode(-1e6);
      }

      // Fill solution vectors by evaluating mesh functions on sampling mesh
      for (const auto var_num : index_range(_mesh_functions[sys_num]))
      {
        // we serialized the mesh and the solution vector, we might as well just do this only on
        // processor 0.
        if (processor_id() > 0)
          break;

        const auto original_var_num = _variable_numbers_in_system[sys_num][var_num];
        const FEType & fe_type = source_sys.variable_type(original_var_num);
        // Loop over the mesh, nodes for nodal data, elements for element data
        if (isOversampledAsNodal(fe_type))
        {
          for (const auto & node : _mesh_ptr->getMesh().node_ptr_range())
          {
            if (node->n_dofs(sys_num, var_num))
            {
              const auto value = (*_mesh_functions[sys_num][var_num])(*node - _position);
              if (value != -1e6)
                dest_sys.solution->set(node->dof_number(sys_num, var_num, /*comp=*/0), value);
              else
                mooseDoOnce(mooseWarning(
                    "Sampling at location ",
                    *node - _position,
                    " was outside the problem mesh.\nThis message will not be repeated"));
            }
          }
        }
        else
          for (const auto & elem : _mesh_ptr->getMesh().active_element_ptr_range())
          {
            if (elem->n_dofs(sys_num, var_num))
            {
              const auto value =
                  (*_mesh_functions[sys_num][var_num])(elem->true_centroid() - _position);
              if (value != -1e6)
                dest_sys.solution->set(elem->dof_number(sys_num, var_num, /*comp=*/0), value);
              else
                mooseDoOnce(mooseWarning(
                    "Sampling at location ",
                    elem->true_centroid() - _position,
                    " was outside the problem mesh.\nThis message will not be repeated."));
            }
          }
      }

      // We don't really need to do that
      dest_sys.solution->close();
    }
  }

  // Set this to false so that new output files are not created, since the oversampled mesh
  // doesn't actually change
  _oversample_mesh_changed = false;
}

void
OversampleOutput::cloneMesh()
{
  // Create the new mesh from a file
  if (isParamValid("file"))
  {
    InputParameters mesh_params = _app.getFactory().getValidParams("FileMesh");
    mesh_params.applyParameters(parameters(), {}, true);
    mesh_params.set<bool>("nemesis") = false;
    _cloned_mesh_ptr =
        _app.getFactory().createUnique<MooseMesh>("FileMesh", "output_problem_mesh", mesh_params);
    _cloned_mesh_ptr->allowRecovery(false); // We actually want to reread the initial mesh
    _cloned_mesh_ptr->init();
    _cloned_mesh_ptr->prepare(/*mesh_to_clone=*/nullptr);
    _cloned_mesh_ptr->meshChanged();
  }

  // Clone the existing mesh
  else
  {
    if (_app.isRecovering())
      mooseWarning("Recovering or Restarting with Oversampling may not work (especially with "
                   "adapted meshes)!!  Refs #2295");
    _cloned_mesh_ptr = _mesh_ptr->safeClone();
  }

  // Remove unspecified blocks
  if (isParamValid("block"))
  {
    // Remove all elements not in the blocks
    const auto & blocks_to_keep_names = getParam<std::vector<SubdomainName>>("block");
    const auto & blocks_to_keep = _cloned_mesh_ptr->getSubdomainIDs(blocks_to_keep_names);
    for (const auto & elem_ptr : _cloned_mesh_ptr->getMesh().element_ptr_range())
      if (std::find(blocks_to_keep.begin(), blocks_to_keep.end(), elem_ptr->subdomain_id()) ==
          blocks_to_keep.end())
        _cloned_mesh_ptr->getMesh().delete_elem(elem_ptr);

    // Remove isolated nodes
    _cloned_mesh_ptr->getMesh().allow_renumbering(false);
    _cloned_mesh_ptr->getMesh().prepare_for_use();
  }

  // Make sure that the mesh pointer points to the newly cloned mesh
  _mesh_ptr = _cloned_mesh_ptr.get();
}

void
OversampleOutput::setFileBaseInternal(const std::string & file_base)
{
  AdvancedOutput::setFileBaseInternal(file_base);
  // ** DEPRECATED SUPPORT **
  if (getParam<bool>("append_oversample"))
    _file_base += "_oversample";
}

bool
OversampleOutput::isOversampledAsNodal(const FEType & fe_type) const
{
  // This is the same criterion as in MooseVariableData
  const auto continuity = FEInterface::get_continuity(fe_type);
  return (continuity == C_ZERO || continuity == C_ONE);
}
