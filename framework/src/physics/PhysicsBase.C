//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsBase.h"
#include "MooseUtils.h"
#include "FEProblemBase.h"

#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "BlockRestrictable.h"
#include "ActionComponent.h"

InputParameters
PhysicsBase::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Creates all the objects necessary to solve a particular physics");

  params.addParam<std::vector<SubdomainName>>(
      "block", {}, "Blocks (subdomains) that this Physics is active on.");

  MooseEnum transient_options("true false same_as_problem", "same_as_problem");
  params.addParam<MooseEnum>(
      "transient", transient_options, "Whether the physics is to be solved as a transient");

  params.addParam<std::vector<SolverSystemName>>(
      "system_names",
      {"nl0"},
      "Name of the solver system(s) for the variables. If a single name is specified, "
      "that system is used for all solver variables.");

  MooseEnum pc_options("default none", "none");
  params.addParam<MooseEnum>(
      "preconditioning", pc_options, "Which preconditioning to use for this Physics");

  params.addParam<bool>("verbose", false, "Flag to facilitate debugging a Physics");

  // Restart parameters
  params.addParam<bool>("initialize_variables_from_mesh_file",
                        false,
                        "Determines if the variables that are added by the action are initialized"
                        "from the mesh file (only for Exodus format)");
  params.addParam<std::string>(
      "initial_from_file_timestep",
      "LATEST",
      "Gives the time step number (or \"LATEST\") for which to read the Exodus solution");
  params.addParamNamesToGroup("initialize_variables_from_mesh_file initial_from_file_timestep",
                              "Restart from Exodus");
  params.addParamNamesToGroup("active inactive", "Advanced");
  return params;
}

PhysicsBase::PhysicsBase(const InputParameters & parameters)
  : Action(parameters),
    InputParametersChecksUtils<PhysicsBase>(this),
    _system_names(getParam<std::vector<SolverSystemName>>("system_names")),
    _verbose(getParam<bool>("verbose")),
    _preconditioning(getParam<MooseEnum>("preconditioning")),
    _blocks(getParam<std::vector<SubdomainName>>("block")),
    _is_transient(getParam<MooseEnum>("transient"))
{
  checkSecondParamSetOnlyIfFirstOneTrue("initialize_variables_from_mesh_file",
                                        "initial_from_file_timestep");
  prepareCopyVariablesFromMesh();
  addRequiredPhysicsTask("init_physics");
  addRequiredPhysicsTask("copy_vars_physics");
  addRequiredPhysicsTask("check_integrity_early_physics");
}

void
PhysicsBase::act()
{
  mooseDoOnce(checkRequiredTasks());

  if (_current_task == "init_physics")
    initializePhysics();
  else if (_current_task == "add_variable")
    addSolverVariables();
  else if (_current_task == "add_ic")
    addInitialConditions();

  else if (_current_task == "add_kernel")
    addFEKernels();
  else if (_current_task == "add_nodal_kernel")
    addNodalKernels();
  else if (_current_task == "add_fv_kernel" || _current_task == "add_linear_fv_kernel")
    addFVKernels();
  else if (_current_task == "add_dirac_kernel")
    addDiracKernels();
  else if (_current_task == "add_dg_kernel")
    addDGKernels();
  else if (_current_task == "add_scalar_kernel")
    addScalarKernels();
  else if (_current_task == "add_interface_kernel")
    addInterfaceKernels();
  else if (_current_task == "add_fv_ik")
    addFVInterfaceKernels();

  else if (_current_task == "add_bc")
    addFEBCs();
  else if (_current_task == "add_nodal_bc")
    addNodalBCs();
  else if (_current_task == "add_fv_bc" || _current_task == "add_linear_fv_bc")
    addFVBCs();
  else if (_current_task == "add_periodic_bc")
    addPeriodicBCs();
  else if (_current_task == "add_function")
    addFunctions();
  else if (_current_task == "add_user_object")
    addUserObjects();
  else if (_current_task == "add_corrector")
    addCorrectors();

  else if (_current_task == "add_aux_variable")
    addAuxiliaryVariables();
  else if (_current_task == "add_aux_kernel")
    addAuxiliaryKernels();
  else if (_current_task == "add_material")
    addMaterials();
  else if (_current_task == "add_functor_material")
    addFunctorMaterials();

  else if (_current_task == "add_multi_app")
    addMultiApps();
  else if (_current_task == "add_transfer")
    addTransfers();

  else if (_current_task == "add_postprocessor")
    addPostprocessors();
  else if (_current_task == "add_vector_postprocessor")
    addVectorPostprocessors();
  else if (_current_task == "add_reporter")
    addReporters();
  else if (_current_task == "add_output")
    addOutputs();
  else if (_current_task == "add_preconditioning")
    addPreconditioning();
  else if (_current_task == "add_executioner")
    addExecutioner();
  else if (_current_task == "add_executor")
    addExecutors();
  else if (_current_task == "check_integrity_early_physics")
    checkIntegrityEarly();

  // Exodus restart capabilities
  if (_current_task == "copy_vars_physics")
  {
    copyVariablesFromMesh(solverVariableNames(), true);
    if (_aux_var_names.size() > 0)
      copyVariablesFromMesh(auxVariableNames(), false);
  }

  // Lets a derived Physics class implement additional tasks
  actOnAdditionalTasks();
}

void
PhysicsBase::prepareCopyVariablesFromMesh() const
{
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    _app.setExodusFileRestart(true);

  checkSecondParamSetOnlyIfFirstOneTrue("initialize_variables_from_mesh_file",
                                        "initial_from_file_timestep");
}

bool
PhysicsBase::isTransient() const
{
  mooseAssert(_problem, "We don't have a problem yet");
  if (_is_transient == "true")
    return true;
  else if (_is_transient == "false")
    return false;
  else
    return getProblem().isTransient();
}

unsigned int
PhysicsBase::dimension() const
{
  mooseAssert(_mesh, "We dont have a mesh yet");
  mooseAssert(_dim < 4, "Dimension has not been set yet");
  return _dim;
}

void
PhysicsBase::addBlocks(const std::vector<SubdomainName> & blocks)
{
  if (blocks.size())
  {
    _blocks.insert(_blocks.end(), blocks.begin(), blocks.end());
    _dim = _mesh->getBlocksMaxDimension(_blocks);
  }
}

void
PhysicsBase::addBlocksById(const std::vector<SubdomainID> & block_ids)
{
  if (block_ids.size())
  {
    for (const auto bid : block_ids)
      _blocks.push_back(_mesh->getSubdomainName(bid));
    _dim = _mesh->getBlocksMaxDimension(_blocks);
  }
}

void
PhysicsBase::addComponent(const ActionComponent & component)
{
  for (const auto & block : component.blocks())
    _blocks.push_back(block);
}

void
PhysicsBase::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  InputParameters params = getAdditionalRMParams();
  Action::addRelationshipManagers(input_rm_type, params);
}

const ActionComponent &
PhysicsBase::getActionComponent(const ComponentName & comp_name)
{
  return _awh.getAction<ActionComponent>(comp_name);
}

void
PhysicsBase::initializePhysics()
{
  // Annoying edge case. We cannot use ANY_BLOCK_ID for kernels and variables since errors got added
  // downstream for using it, we cannot leave it empty as that sets all objects to not live on any
  // block
  if (isParamSetByUser("block") && _blocks.empty())
    paramError("block",
               "Empty block restriction is not supported. Comment out the Physics if you are "
               "trying to disable it.");

  // Components should have added their blocks already.
  if (_blocks.empty())
    _blocks.push_back("ANY_BLOCK_ID");

  mooseAssert(_mesh, "We should have a mesh to find the dimension");
  if (_blocks.size())
    _dim = _mesh->getBlocksMaxDimension(_blocks);
  else
    _dim = _mesh->dimension();

  // Forward physics verbosity to problem to output the setup
  if (_verbose)
    getProblem().setVerboseProblem(_verbose);

  // If the derived physics need additional initialization very early on
  initializePhysicsAdditional();

  // Check that the systems exist in the Problem
  // TODO: try to add the systems to the problem from here instead
  // NOTE: this must be performed after the "Additional" initialization because the list
  // of systems might have been adjusted once the dimension of the Physics is known
  const auto & problem_nl_systems = getProblem().getNonlinearSystemNames();
  const auto & problem_lin_systems = getProblem().getLinearSystemNames();
  for (const auto & sys_name : _system_names)
    if (std::find(problem_nl_systems.begin(), problem_nl_systems.end(), sys_name) ==
            problem_nl_systems.end() &&
        std::find(problem_lin_systems.begin(), problem_lin_systems.end(), sys_name) ==
            problem_lin_systems.end() &&
        solverVariableNames().size())
      mooseError("System '", sys_name, "' is not found in the Problem");

  // Cache system number as it makes some logic easier
  for (const auto & sys_name : _system_names)
    _system_numbers.push_back(getProblem().solverSysNum(sys_name));
}

void
PhysicsBase::checkIntegrityEarly() const
{
  if (_is_transient == "true" && !getProblem().isTransient())
    paramError("transient", "We cannot solve a physics as transient in a steady problem");

  // Check that there is a system for each variable
  if (_system_names.size() != 1 && _system_names.size() != _solver_var_names.size())
    paramError("system_names",
               "There should be one system name per solver variable (potentially repeated), or a "
               "single system name for all variables. Current you have '" +
                   std::to_string(_system_names.size()) + "' systems specified for '" +
                   std::to_string(_solver_var_names.size()) + "' solver variables.");

  // Check that each variable is present in the expected system
  unsigned int var_i = 0;
  for (const auto & var_name : _solver_var_names)
  {
    const auto & sys_name = _system_names.size() == 1 ? _system_names[0] : _system_names[var_i++];
    if (!_problem->getSolverSystem(_problem->solverSysNum(sys_name)).hasVariable(var_name))
      paramError("system_names",
                 "We expected system '" + sys_name + "' to contain variable '" + var_name +
                     "' but it did not. Make sure the system names closely match the ordering of "
                     "the variables in the Physics.");
  }
}

void
PhysicsBase::copyVariablesFromMesh(const std::vector<VariableName> & variables_to_copy,
                                   bool are_nonlinear)
{
  if (getParam<bool>("initialize_variables_from_mesh_file"))
  {
    mooseInfoRepeated("Adding Exodus restart for " + std::to_string(variables_to_copy.size()) +
                      " variables: " + Moose::stringify(variables_to_copy));
    // TODO Check that the variable types and orders are actually supported for exodus restart
    for (const auto i : index_range(variables_to_copy))
    {
      SystemBase & system =
          are_nonlinear ? getProblem().getNonlinearSystemBase(
                              _system_numbers.size() == 1 ? _system_numbers[0] : _system_numbers[i])
                        : getProblem().systemBaseAuxiliary();
      const auto & var_name = variables_to_copy[i];
      system.addVariableToCopy(
          var_name, var_name, getParam<std::string>("initial_from_file_timestep"));
    }
  }
}

bool
PhysicsBase::variableExists(const VariableName & var_name, bool error_if_aux) const
{
  if (error_if_aux && _problem->getAuxiliarySystem().hasVariable(var_name))
    mooseError("Variable '",
               var_name,
               "' is supposed to be nonlinear for physics '",
               name(),
               "' but it is already defined as auxiliary");
  else if (_problem->hasVariable(var_name))
    return true;
  else
    return false;
}

const SolverSystemName &
PhysicsBase::getSolverSystem(unsigned int variable_index) const
{
  mooseAssert(!_system_names.empty(), "We should have a solver system name");
  if (_system_names.size() == 1)
    return _system_names[0];
  else
    // We trust that the system names and the variable names match one-to-one as it is enforced by
    // the checkIntegrityEarly() routine.
    return _system_names[variable_index];
}

const SolverSystemName &
PhysicsBase::getSolverSystem(const VariableName & var_name) const
{
  mooseAssert(!_system_names.empty(), "We should have a solver system name");
  // No need to look if only one system for the Physics
  if (_system_names.size() == 1)
    return _system_names[0];

  // We trust that the system names and the variable names match one-to-one as it is enforced by the
  // checkIntegrityEarly() routine.
  for (const auto variable_index : index_range(_solver_var_names))
    if (var_name == _solver_var_names[variable_index])
      return _system_names[variable_index];
  mooseError("Variable '", var_name, "' was not found within the Physics solver variables.");
}

void
PhysicsBase::checkRequiredTasks() const
{
  const auto registered_tasks = _action_factory.getTasksByAction(type());

  // Check for missing tasks
  for (const auto & required_task : _required_tasks)
    if (!registered_tasks.count(required_task))
      mooseWarning("Task '" + required_task +
                   "' has been declared as required by a Physics parent class of derived class '" +
                   type() +
                   "' but this task is not registered to the derived class. Registered tasks for "
                   "this Physics are: " +
                   Moose::stringify(registered_tasks));
}

void
PhysicsBase::assignBlocks(InputParameters & params, const std::vector<SubdomainName> & blocks) const
{
  // We only set the blocks if we don't have `ANY_BLOCK_ID` defined because the subproblem
  // (through the mesh) errors out if we use this keyword during the addVariable/Kernel
  // functions
  if (std::find(blocks.begin(), blocks.end(), "ANY_BLOCK_ID") == blocks.end())
    params.set<std::vector<SubdomainName>>("block") = blocks;
  if (blocks.empty())
    mooseInfoRepeated("Empty block restriction assigned to an object created by Physics '" +
                      name() + "'.\n Did you mean to do this?");
}

bool
PhysicsBase::checkBlockRestrictionIdentical(const std::string & object_name,
                                            const std::vector<SubdomainName> & blocks,
                                            bool error_if_not_identical) const
{
  // If identical, we can return fast
  if (_blocks == blocks)
    return true;
  // If one is block restricted to anywhere and the other is block restricted to anywhere manually
  if ((std::find(_blocks.begin(), _blocks.end(), "ANY_BLOCK_ID") != _blocks.end() &&
       allMeshBlocks(blocks)) ||
      (std::find(blocks.begin(), blocks.end(), "ANY_BLOCK_ID") != blocks.end() &&
       allMeshBlocks(_blocks)))
    return true;

  // Copy, sort and unique is the only way to check that they are actually the same
  auto copy_blocks = _blocks;
  auto copy_blocks_other = blocks;
  std::sort(copy_blocks.begin(), copy_blocks.end());
  copy_blocks.erase(unique(copy_blocks.begin(), copy_blocks.end()), copy_blocks.end());
  std::sort(copy_blocks_other.begin(), copy_blocks_other.end());
  copy_blocks_other.erase(unique(copy_blocks_other.begin(), copy_blocks_other.end()),
                          copy_blocks_other.end());

  if (copy_blocks == copy_blocks_other)
    return true;
  std::vector<SubdomainName> diff;
  std::set_difference(copy_blocks.begin(),
                      copy_blocks.end(),
                      copy_blocks_other.begin(),
                      copy_blocks_other.end(),
                      std::inserter(diff, diff.begin()));
  if (error_if_not_identical)
    mooseError("Physics '",
               name(),
               "' and object '",
               object_name,
               "' have different block restrictions.\nPhysics: ",
               Moose::stringify(_blocks),
               "\nObject: ",
               Moose::stringify(blocks),
               "\nDifference: ",
               Moose::stringify(diff));
  else
    return false;
}

bool
PhysicsBase::allMeshBlocks(const std::vector<SubdomainName> & blocks) const
{
  mooseAssert(_mesh, "The mesh should exist already");
  for (const auto mesh_block : _mesh->meshSubdomains())
    if (std::find(blocks.begin(), blocks.end(), _mesh->getSubdomainName(mesh_block)) ==
        blocks.end())
      return false;
  return true;
}
