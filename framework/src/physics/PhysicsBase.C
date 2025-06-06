//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "InitialConditionBase.h"
#include "FVInitialConditionBase.h"
#include "MooseVariableScalar.h"
#include "LinearSystem.h"

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

  params.addParam<bool>("verbose", false, "Flag to facilitate debugging a Physics");

  // Numerical solve parameters
  params.addParam<std::vector<SolverSystemName>>(
      "system_names",
      {"nl0"},
      "Name of the solver system(s) for the variables. If a single name is specified, "
      "that system is used for all solver variables.");
  MooseEnum pc_options("default defer", "defer");
  params.addParam<MooseEnum>("preconditioning",
                             pc_options,
                             "Which preconditioning to use/add for this Physics, or whether to "
                             "defer to the Preconditioning block, or another Physics");

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

  // Options to turn off tasks
  params.addParam<bool>(
      "dont_create_solver_variables", false, "Whether to skip the 'add_variable' task");
  params.addParam<bool>("dont_create_ics", false, "Whether to skip the 'add_ic' task");
  params.addParam<bool>(
      "dont_create_kernels", false, "Whether to skip the 'add_kernel' task for each kernel type");
  params.addParam<bool>("dont_create_bcs",
                        false,
                        "Whether to skip the 'add_bc' task for each boundary condition type");
  params.addParam<bool>("dont_create_functions", false, "Whether to skip the 'add_function' task");
  params.addParam<bool>(
      "dont_create_aux_variables", false, "Whether to skip the 'add_aux_variable' task");
  params.addParam<bool>(
      "dont_create_aux_kernels", false, "Whether to skip the 'add_aux_kernel' task");
  params.addParam<bool>("dont_create_materials",
                        false,
                        "Whether to skip the 'add_material' task for each material type");
  params.addParam<bool>(
      "dont_create_user_objects",
      false,
      "Whether to skip the 'add_user_object' task. This does not apply to UserObject derived "
      "classes being created on a different task (for example: postprocessors, VPPs, correctors)");
  params.addParam<bool>(
      "dont_create_correctors", false, "Whether to skip the 'add_correctors' task");
  params.addParam<bool>(
      "dont_create_postprocessors", false, "Whether to skip the 'add_postprocessors' task");
  params.addParam<bool>("dont_create_vectorpostprocessors",
                        false,
                        "Whether to skip the 'add_vectorpostprocessors' task");
  params.addParamNamesToGroup(
      "dont_create_solver_variables dont_create_ics dont_create_kernels dont_create_bcs "
      "dont_create_functions dont_create_aux_variables dont_create_aux_kernels "
      "dont_create_materials dont_create_user_objects dont_create_correctors "
      "dont_create_postprocessors dont_create_vectorpostprocessors",
      "Reduce Physics object creation");

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

  // Lets a derived Physics class implement additional tasks
  actOnAdditionalTasks();

  // Initialization and variables
  if (_current_task == "init_physics")
    initializePhysics();
  else if (_current_task == "add_variable" && !getParam<bool>("dont_create_solver_variables"))
    addSolverVariables();
  else if (_current_task == "add_ic" && !getParam<bool>("dont_create_ics"))
    addInitialConditions();

  // Kernels
  else if (_current_task == "add_kernel" && !getParam<bool>("dont_create_kernels"))
    addFEKernels();
  else if (_current_task == "add_nodal_kernel" && !getParam<bool>("dont_create_kernels"))
    addNodalKernels();
  else if ((_current_task == "add_fv_kernel" || _current_task == "add_linear_fv_kernel") &&
           !getParam<bool>("dont_create_kernels"))
    addFVKernels();
  else if (_current_task == "add_dirac_kernel" && !getParam<bool>("dont_create_kernels"))
    addDiracKernels();
  else if (_current_task == "add_dg_kernel" && !getParam<bool>("dont_create_kernels"))
    addDGKernels();
  else if (_current_task == "add_scalar_kernel" && !getParam<bool>("dont_create_kernels"))
    addScalarKernels();
  else if (_current_task == "add_interface_kernel" && !getParam<bool>("dont_create_kernels"))
    addInterfaceKernels();
  else if (_current_task == "add_fv_ik" && !getParam<bool>("dont_create_kernels"))
    addFVInterfaceKernels();

  // Boundary conditions
  else if (_current_task == "add_bc" && !getParam<bool>("dont_create_bcs"))
    addFEBCs();
  else if (_current_task == "add_nodal_bc" && !getParam<bool>("dont_create_bcs"))
    addNodalBCs();
  else if ((_current_task == "add_fv_bc" || _current_task == "add_linear_fv_bc") &&
           !getParam<bool>("dont_create_bcs"))
    addFVBCs();
  else if (_current_task == "add_periodic_bc" && !getParam<bool>("dont_create_bcs"))
    addPeriodicBCs();

  // Auxiliary quantities
  else if (_current_task == "add_function" && !getParam<bool>("dont_create_functions"))
    addFunctions();
  else if (_current_task == "add_aux_variable" && !getParam<bool>("dont_create_aux_variables"))
    addAuxiliaryVariables();
  else if (_current_task == "add_aux_kernel" && !getParam<bool>("dont_create_aux_kernels"))
    addAuxiliaryKernels();
  else if (_current_task == "add_material" && !getParam<bool>("dont_create_materials"))
    addMaterials();
  else if (_current_task == "add_functor_material" && !getParam<bool>("dont_create_materials"))
    addFunctorMaterials();

  // Multiapp
  else if (_current_task == "add_multi_app")
    addMultiApps();
  else if (_current_task == "add_transfer")
    addTransfers();

  // User objects and output
  else if (_current_task == "add_user_object" && !getParam<bool>("dont_create_user_objects"))
    addUserObjects();
  else if (_current_task == "add_corrector" && !getParam<bool>("dont_create_correctors"))
    addCorrectors();
  else if (_current_task == "add_postprocessor" && !getParam<bool>("dont_create_postprocessors"))
    addPostprocessors();
  else if (_current_task == "add_vector_postprocessor" &&
           !getParam<bool>("dont_create_vectorpostprocessors"))
    addVectorPostprocessors();
  else if (_current_task == "add_reporter")
    addReporters();
  else if (_current_task == "add_output")
    addOutputs();

  // Equation solver-related tasks
  else if (_current_task == "add_preconditioning")
    addPreconditioning();
  else if (_current_task == "add_executioner")
    addExecutioner();
  else if (_current_task == "add_executor")
    addExecutors();

  // Checks
  else if (_current_task == "check_integrity_early_physics")
    checkIntegrityEarly();
  else if (_current_task == "check_integrity")
    checkIntegrity();

  // Exodus restart capabilities
  if (_current_task == "copy_vars_physics")
  {
    copyVariablesFromMesh(solverVariableNames(), true);
    if (_aux_var_names.size() > 0)
      copyVariablesFromMesh(auxVariableNames(), false);
  }
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

std::set<SubdomainID>
PhysicsBase::getSubdomainIDs(const std::set<SubdomainName> & blocks) const
{
  const bool not_block_restricted =
      (std::find(blocks.begin(), blocks.end(), "ANY_BLOCK_ID") != blocks.end()) ||
      allMeshBlocks(blocks);
  mooseAssert(_mesh, "Should have a mesh");
  // use a set for simplicity. Note that subdomain names are unique, except maybe the empty one,
  // which cannot be specified by the user to the Physics.
  // MooseMesh::getSubdomainIDs cannot deal with the 'ANY_BLOCK_ID' name
  std::set<SubdomainID> block_ids_set =
      not_block_restricted ? _mesh->meshSubdomains() : _mesh->getSubdomainIDs(blocks);
  return block_ids_set;
}

std::vector<std::string>
PhysicsBase::getSubdomainNamesAndIDs(const std::set<SubdomainID> & blocks) const
{
  mooseAssert(_mesh, "Should have a mesh");
  std::vector<std::string> sub_names_ids;
  sub_names_ids.reserve(blocks.size());
  for (const auto bid : blocks)
  {
    const auto bname = _mesh->getSubdomainName(bid);
    sub_names_ids.push_back((bname.empty() ? "(unnamed)" : bname) + " (" + std::to_string(bid) +
                            ")");
  }
  return sub_names_ids;
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
PhysicsBase::getActionComponent(const ComponentName & comp_name) const
{
  return _awh.getAction<ActionComponent>(comp_name);
}

void
PhysicsBase::initializePhysics()
{
  // Annoying edge case. We cannot use ANY_BLOCK_ID for kernels and variables since errors got
  // added downstream for using it, we cannot leave it empty as that sets all objects to not live
  // on any block
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
    if (!_problem->getSolverSystem(_problem->solverSysNum(sys_name)).hasVariable(var_name) &&
        !_problem->getSolverSystem(_problem->solverSysNum(sys_name)).hasScalarVariable(var_name))
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
  else
    return _problem->hasVariable(var_name);
}

bool
PhysicsBase::solverVariableExists(const VariableName & var_name) const
{
  return _problem->hasSolverVariable(var_name);
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

  // We trust that the system names and the variable names match one-to-one as it is enforced by
  // the checkIntegrityEarly() routine.
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
  // Try to return faster without examining every single block
  if (std::find(blocks.begin(), blocks.end(), "ANY_BLOCK_ID") != blocks.end())
    return true;
  else if (blocks.size() != _mesh->meshSubdomains().size())
    return false;

  for (const auto mesh_block : _mesh->meshSubdomains())
  {
    const auto & subdomain_name = _mesh->getSubdomainName(mesh_block);
    // Check subdomain name
    if (!subdomain_name.empty() &&
        std::find(blocks.begin(), blocks.end(), subdomain_name) == blocks.end())
      return false;
    // no subdomain name, check the IDs being used as names instead
    else if (std::find(blocks.begin(), blocks.end(), std::to_string(mesh_block)) == blocks.end())
      return false;
  }
  return true;
}

bool
PhysicsBase::allMeshBlocks(const std::set<SubdomainName> & blocks) const
{
  std::vector<SubdomainName> blocks_vec(blocks.begin(), blocks.end());
  return allMeshBlocks(blocks_vec);
}

void
PhysicsBase::addPetscPairsToPetscOptions(
    const std::vector<std::pair<MooseEnumItem, std::string>> & petsc_pair_options)
{
  Moose::PetscSupport::PetscOptions & po = _problem->getPetscOptions();
  for (const auto solver_sys_num : _system_numbers)
    Moose::PetscSupport::addPetscPairsToPetscOptions(
        petsc_pair_options,
        _problem->mesh().dimension(),
        _problem->getSolverSystem(solver_sys_num).prefix(),
        *this,
        po);
}

bool
PhysicsBase::isVariableFV(const VariableName & var_name) const
{
  const auto var = &_problem->getVariable(0, var_name);
  return var->isFV();
}

bool
PhysicsBase::isVariableScalar(const VariableName & var_name) const
{
  return _problem->hasScalarVariable(var_name);
}

bool
PhysicsBase::shouldCreateVariable(const VariableName & var_name,
                                  const std::vector<SubdomainName> & blocks,
                                  const bool error_if_aux)
{
  if (!variableExists(var_name, error_if_aux))
    return true;
  // check block restriction
  auto & var = _problem->getVariable(0, var_name);
  const bool not_block_restricted =
      (std::find(blocks.begin(), blocks.end(), "ANY_BLOCK_ID") != blocks.end()) ||
      allMeshBlocks(blocks);
  if (!var.blockRestricted() || (!not_block_restricted && var.hasBlocks(blocks)))
    return false;

  // This is an edge case, which might warrant a warning
  if (allMeshBlocks(var.blocks()) && not_block_restricted)
    return false;
  else
    mooseError("Variable '" + var_name + "' already exists with subdomain restriction '" +
               Moose::stringify(var.blocks()) + "' which does not include the subdomains '" +
               Moose::stringify(blocks) + "', required for this Physics.");
}

bool
PhysicsBase::shouldCreateIC(const VariableName & var_name,
                            const std::vector<SubdomainName> & blocks,
                            const bool ic_is_default_ic,
                            const bool error_if_already_defined) const
{
  // Handle recover
  if (ic_is_default_ic && (_app.isRestarting() || _app.isRecovering()))
    return false;
  // do not set initial conditions if we are loading fields from the mesh file
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return false;
  // Different type of ICs, not block restrictable
  mooseAssert(!isVariableScalar(var_name), "shouldCreateIC not implemented for scalar variables");

  // Process the desired block restriction into a set of subdomain IDs
  std::set<SubdomainName> blocks_set(blocks.begin(), blocks.end());
  const auto blocks_ids_set = getSubdomainIDs(blocks_set);

  // Check whether there are any ICs for this variable already in the problem
  std::set<SubdomainID> blocks_ids_covered;
  bool has_all_blocks;
  if (isVariableFV(var_name))
    has_all_blocks = _problem->getFVInitialConditionWarehouse().hasObjectsForVariableAndBlocks(
        var_name, blocks_ids_set, blocks_ids_covered, /*tid =*/0);
  else
    has_all_blocks = _problem->getInitialConditionWarehouse().hasObjectsForVariableAndBlocks(
        var_name, blocks_ids_set, blocks_ids_covered, /*tid =*/0);

  const bool has_some_blocks = !blocks_ids_covered.empty();
  if (!has_some_blocks)
    return true;

  if (has_all_blocks)
  {
    if (error_if_already_defined)
      mooseError("ICs for variable '" + var_name + "' have already been defined for blocks '" +
                 Moose::stringify(blocks) + "'.");
    else
      return false;
  }

  // Partial overlap between Physics is not implemented.
  mooseError("There is a partial overlap between the subdomains covered by pre-existing initial "
             "conditions (ICs), defined on blocks (ids): " +
             Moose::stringify(getSubdomainNamesAndIDs(blocks_ids_covered)) +
             "\n and a newly created IC for variable " + var_name +
             ", to be defined on blocks: " + Moose::stringify(blocks) +
             ".\nWe should be creating the Physics' IC only for non-covered blocks. This is not "
             "implemented at this time.");
}

bool
PhysicsBase::shouldCreateTimeDerivative(const VariableName & var_name,
                                        const std::vector<SubdomainName> & blocks,
                                        const bool error_if_already_defined) const
{
  // Follow the transient setting of the Physics
  if (!isTransient())
    return false;

  // Variable is either nonlinear (FV/FE), nodal nonlinear (field of ODEs), linear, or scalar.
  // The warehouses hosting the time kernels are different for each of these types
  // Different type of time derivatives, not block restrictable
  mooseAssert(!isVariableScalar(var_name),
              "shouldCreateTimeDerivative not implemented for scalar variables");
  mooseAssert(!_problem->hasAuxiliaryVariable(var_name),
              "Should not be called with auxiliary variables");

  // Get solver system type
  const auto var = &_problem->getVariable(0, var_name);
  const auto var_id = var->number();
  const auto sys_num = var->sys().number();
  const auto time_vector_tag =
      (sys_num < _problem->numNonlinearSystems())
          ? var->sys().timeVectorTag()
          // this is not quite correct. Many kernels can contribute to RHS time vector on paper
          : dynamic_cast<LinearSystem *>(&var->sys())->rightHandSideTimeVectorTag();

  // We just use the warehouse, it should cover every time derivative object type
  bool all_blocks_covered = true;
  std::set<SubdomainID> blocks_ids_covered;
  // we examine subdomain by subdomain, because mutiple kernels could be covering every block in
  // the 'blocks' parameter
  for (const auto & block : blocks)
  {
    std::vector<MooseObject *> time_kernels;
    if (block != "ANY_BLOCK_ID")
    {
      const auto bid = _mesh->getSubdomainID(block);
      _problem->theWarehouse()
          .query()
          .template condition<AttribSysNum>(sys_num)
          .template condition<AttribVar>(var_id)
          .template condition<AttribSubdomains>(bid)
          // we use the time tag as a proxy for time derivatives
          .template condition<AttribVectorTags>(time_vector_tag)
          .queryInto(time_kernels);
    }
    else
      _problem->theWarehouse()
          .query()
          .template condition<AttribSysNum>(sys_num)
          .template condition<AttribVar>(var_id)
          // we use the time tag as a proxy for time derivatives
          .template condition<AttribVectorTags>(time_vector_tag)
          .queryInto(time_kernels);

    if (time_kernels.size())
    {
      if (block == "ANY_BLOCK_ID")
      {
        for (const auto & time_kernel : time_kernels)
          if (const auto blk = dynamic_cast<BlockRestrictable *>(time_kernel))
            blocks_ids_covered.insert(blk->blockIDs().begin(), blk->blockIDs().end());
      }
      else
        blocks_ids_covered.insert(_mesh->getSubdomainID(block));
    }
    else
      all_blocks_covered = false;
  }

  // From the set of covered blocks, see if the blocks we needed are found
  if (all_blocks_covered)
  {
    std::set<SubdomainName> blocks_set(blocks.begin(), blocks.end());
    const auto blocks_ids = getSubdomainIDs(blocks_set);
    if (blocks_ids != blocks_ids_covered)
      all_blocks_covered = false;
  }
  const bool has_some_blocks = !blocks_ids_covered.empty();
  if (!has_some_blocks)
    return true;
  if (all_blocks_covered)
  {
    if (error_if_already_defined)
      mooseError("A time kernel for variable '" + var_name +
                 "' has already been defined on blocks '" + Moose::stringify(blocks) + "'.");
    else
      return false;
  }

  // Partial overlap between Physics is not implemented.
  mooseError("There is a partial overlap between the subdomains covered by pre-existing time "
             "derivative kernel(s), defined on blocks (ids): " +
             Moose::stringify(getSubdomainNamesAndIDs(blocks_ids_covered)) +
             "\nand a newly created time derivative kernel for variable " + var_name +
             ", to be defined on blocks: " + Moose::stringify(blocks) +
             ".\nWe should be creating the Physics' time derivative only for non-covered "
             "blocks. This is not implemented at this time.");
}

void
PhysicsBase::reportPotentiallyMissedParameters(const std::vector<std::string> & param_names,
                                               const std::string & object_type) const
{
  std::vector<std::string> defaults_unused;
  std::vector<std::string> user_values_unused;
  for (const auto & param : param_names)
  {
    if (isParamSetByUser(param))
      user_values_unused.push_back(param);
    else if (isParamValid(param))
      defaults_unused.push_back(param);
  }
  if (defaults_unused.size() && _verbose)
    mooseInfoRepeated("Defaults for parameters '" + Moose::stringify(defaults_unused) +
                      "' for object of type '" + object_type +
                      "' were not used because the object was not created by this Physics.");
  if (user_values_unused.size())
  {
    if (_app.unusedFlagIsWarning())
      mooseWarning(
          "User-specifed values for parameters '" + Moose::stringify(user_values_unused) +
          "' for object of type '" + object_type +
          "' were not used because the corresponding object was not created by this Physics.");
    else if (_app.unusedFlagIsError())
      mooseError(
          "User-specified values for parameters '" + Moose::stringify(user_values_unused) +
          "' for object of type '" + object_type +
          "' were not used because the corresponding object was not created by this Physics.");
  }
}
