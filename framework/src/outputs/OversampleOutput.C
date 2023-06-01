//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include "FileMesh.h"
#include "MooseApp.h"

#include "libmesh/distributed_mesh.h"
#include "libmesh/equation_systems.h"
#include "libmesh/mesh_function.h"
#include "libmesh/explicit_system.h"

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

  // **** DEPRECATED AND REMOVED PARAMETERS ****
  params.addDeprecatedParam<bool>("oversample",
                                  false,
                                  "Set to true to enable oversampling",
                                  "This parameter is no longer active, simply set 'refinements' to "
                                  "a value greater than zero to evoke oversampling");
  params.addDeprecatedParam<bool>("append_oversample",
                                  false,
                                  "Append '_oversample' to the output file base",
                                  "This parameter is no longer operational, to append "
                                  "'_oversample' utilize the output block name or 'file_base'");

  // 'Oversampling' Group
  params.addParamNamesToGroup("refinements position file", "Oversampling");

  return params;
}

OversampleOutput::OversampleOutput(const InputParameters & parameters)
  : AdvancedOutput(parameters),
    _refinements(getParam<unsigned int>("refinements")),
    _oversample(_refinements > 0 || isParamValid("file")),
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

  // Call the output method (this has the file checking built in b/c OversampleOutput is a
  // FileOutput)
  if (shouldOutput(type))
  {
    TIME_SECTION("outputStep", 1);
    updateOversample();
    output(type);
  }
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
  DistributedMesh * dist_mesh = dynamic_cast<DistributedMesh *>(&source_es.get_mesh());
  if (dist_mesh)
  {
    for (auto & elem : dist_mesh->active_local_element_ptr_range())
      dist_mesh->add_extra_ghost_elem(elem);
  }

  // Initialize the _mesh_functions vector
  unsigned int num_systems = source_es.n_systems();
  _mesh_functions.resize(num_systems);

  // Loop over the number of systems
  for (unsigned int sys_num = 0; sys_num < num_systems; sys_num++)
  {
    // Reference to the current system
    System & source_sys = source_es.get_system(sys_num);

    // Add the system to the new EquationsSystems
    ExplicitSystem & dest_sys = _oversample_es->add_system<ExplicitSystem>(source_sys.name());

    // Loop through the variables in the System
    unsigned int num_vars = source_sys.n_vars();
    if (num_vars > 0)
    {
      _mesh_functions[sys_num].resize(num_vars);
      _serialized_solution = NumericVector<Number>::build(_communicator);
      _serialized_solution->init(source_sys.n_dofs(), false, SERIAL);

      // Need to pull down a full copy of this vector on every processor so we can get values in
      // parallel
      source_sys.solution->localize(*_serialized_solution);

      // Add the variables to the system... simultaneously creating MeshFunctions for them.
      for (unsigned int var_num = 0; var_num < num_vars; var_num++)
      {
        // Add the variable, allow for first and second lagrange
        const FEType & fe_type = source_sys.variable_type(var_num);
        FEType second(SECOND, LAGRANGE);
        if (fe_type == second)
          dest_sys.add_variable(source_sys.variable_name(var_num), second);
        else
          dest_sys.add_variable(source_sys.variable_name(var_num), FEType());
      }
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

  // Get a reference to actual equation system
  EquationSystems & source_es = _problem_ptr->es();

  // Loop throuch each system
  for (unsigned int sys_num = 0; sys_num < source_es.n_systems(); ++sys_num)
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
      for (unsigned int var_num = 0; var_num < _mesh_functions[sys_num].size(); ++var_num)
      {

        // If the mesh has change the MeshFunctions need to be re-built, otherwise simply clear it
        // for re-initialization
        if (!_mesh_functions[sys_num][var_num] || _oversample_mesh_changed)
          _mesh_functions[sys_num][var_num] = std::make_unique<MeshFunction>(
              source_es, *_serialized_solution, source_sys.get_dof_map(), var_num);
        else
          _mesh_functions[sys_num][var_num]->clear();

        // Initialize the MeshFunctions for application to the oversampled solution
        _mesh_functions[sys_num][var_num]->init();
      }

      // Now loop over the nodes of the oversampled mesh setting values for each variable.
      for (const auto & node : as_range(_mesh_ptr->localNodesBegin(), _mesh_ptr->localNodesEnd()))
        for (unsigned int var_num = 0; var_num < _mesh_functions[sys_num].size(); ++var_num)
          if (node->n_dofs(sys_num, var_num))
            dest_sys.solution->set(node->dof_number(sys_num, var_num, 0),
                                   (*_mesh_functions[sys_num][var_num])(
                                       *node - _position)); // 0 value is for component

      dest_sys.solution->close();
    }
  }

  // Set this to false so that new output files are not created, since the oversampled mesh doesn't
  // actually change
  _oversample_mesh_changed = false;
}

void
OversampleOutput::cloneMesh()
{
  // Create the new mesh from a file
  if (isParamValid("file"))
  {
    InputParameters mesh_params = emptyInputParameters();
    mesh_params += _mesh_ptr->parameters();
    mesh_params.set<MeshFileName>("file") = getParam<MeshFileName>("file");
    mesh_params.set<bool>("nemesis") = false;
    mesh_params.set<bool>("skip_partitioning") = false;
    mesh_params.set<std::string>("_object_name") = "output_problem_mesh";
    _cloned_mesh_ptr = std::make_unique<FileMesh>(mesh_params);
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
