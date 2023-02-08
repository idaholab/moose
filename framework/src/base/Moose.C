//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/petsc_macro.h"
#include "libmesh/libmesh_config.h"

#include "Moose.h"
#include "MooseApp.h"

#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "AuxiliarySystem.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "Syntax.h"
#include "MooseSyntax.h"
#include "ExecFlagRegistry.h"

#include <unistd.h>

const ExecFlagType EXEC_NONE = registerDefaultExecFlag("NONE");
const ExecFlagType EXEC_INITIAL = registerDefaultExecFlag("INITIAL");
const ExecFlagType EXEC_LINEAR = registerDefaultExecFlag("LINEAR");
const ExecFlagType EXEC_NONLINEAR = registerDefaultExecFlag("NONLINEAR");
const ExecFlagType EXEC_TIMESTEP_END = registerDefaultExecFlag("TIMESTEP_END");
const ExecFlagType EXEC_TIMESTEP_BEGIN = registerDefaultExecFlag("TIMESTEP_BEGIN");
const ExecFlagType EXEC_MULTIAPP_FIXED_POINT_END =
    registerDefaultExecFlag("MULTIAPP_FIXED_POINT_END");
const ExecFlagType EXEC_MULTIAPP_FIXED_POINT_BEGIN =
    registerDefaultExecFlag("MULTIAPP_FIXED_POINT_BEGIN");
const ExecFlagType EXEC_FINAL = registerDefaultExecFlag("FINAL");
const ExecFlagType EXEC_FORCED = registerExecFlag("FORCED");
const ExecFlagType EXEC_FAILED = registerExecFlag("FAILED");
const ExecFlagType EXEC_CUSTOM = registerDefaultExecFlag("CUSTOM");
const ExecFlagType EXEC_SUBDOMAIN = registerExecFlag("SUBDOMAIN");
const ExecFlagType EXEC_ALWAYS = registerDefaultExecFlag("ALWAYS");
const ExecFlagType EXEC_PRE_DISPLACE = registerExecFlag("PRE_DISPLACE");
const ExecFlagType EXEC_SAME_AS_MULTIAPP = registerExecFlag("SAME_AS_MULTIAPP");
const ExecFlagType EXEC_PRE_MULTIAPP_SETUP = registerExecFlag("PRE_MULTIAPP_SETUP");
const ExecFlagType EXEC_TRANSFER = registerExecFlag("TRANSFER");
const ExecFlagType EXEC_PRE_KERNELS = registerExecFlag("PRE_KERNELS");

namespace Moose
{

void associateSyntaxInner(Syntax & syntax, ActionFactory & action_factory);

void
registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  registerObjects(f, {"MooseApp"});
  associateSyntaxInner(s, af);
  registerActions(s, af, {"MooseApp"});
  registerDataFilePath();
}

void
registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  registerObjects(factory, {"MooseApp"});
}

void
registerObjects(Factory & factory, const std::set<std::string> & obj_labels)
{
  Registry::registerObjectsTo(factory, obj_labels);
}

void
addActionTypes(Syntax & syntax)
{
  /**
   * The (optional) last param here indicates whether the task should trigger an Action auto-build.
   * If a task is marked as "true". Then MOOSE will attempt to build the associated Action if one is
   * not supplied by some other means (usually through the input file or custom Action). Only
   * Actions that do not have required parameters and have defaults for all optional parameters can
   * be built automatically (See ActionWarehouse.C).
   *
   * Note: Many of the actions in the "Minimal Problem" section are marked as false.  However, we
   * can generally force creation of these "Action"s as needed by registering them to syntax that we
   * expect to see even if those "Action"s  don't normally pick up parameters from the input file.
   */

  // clang-format off
  /**************************/
  /**** Register Actions ****/
  /**************************/
  registerMooseObjectTask("create_problem",               Problem,                false);
  registerMooseObjectTask("setup_executioner",            Executioner,            false);
  registerMooseObjectTask("read_executor",                Executor,               false);
  registerTask("add_executor", true);

  // This task does not construct an object, but it needs all of the parameters that
  // would normally be used to construct an object.
  registerMooseObjectTask("determine_system_type",        Executioner,            true);

  registerMooseObjectTask("setup_mesh",                   MooseMesh,              false);
  registerMooseObjectTask("set_mesh_base",                MooseMesh,              false);
  registerMooseObjectTask("init_mesh",                    MooseMesh,              false);
  registerMooseObjectTask("add_mesh_generator",           MeshGenerator,          false);
  registerTask("create_added_mesh_generators", true);
  registerMooseObjectTask("append_mesh_generator",        MeshGenerator,          false);

  registerMooseObjectTask("add_kernel",                   Kernel,                 false);
  appendMooseObjectTask  ("add_kernel",                   EigenKernel);
  appendMooseObjectTask  ("add_kernel",                   VectorKernel);
  appendMooseObjectTask  ("add_kernel",                   ArrayKernel);

  registerMooseObjectTask("add_variable",                 MooseVariableBase,      false);
  registerMooseObjectTask("add_aux_variable",             MooseVariableBase,      false);
  registerMooseObjectTask("add_elemental_field_variable", MooseVariableBase,      false);

  registerMooseObjectTask("add_nodal_kernel",             NodalKernel,            false);

  registerMooseObjectTask("add_material",                 MaterialBase,           false);
  registerMooseObjectTask("add_bc",                       BoundaryCondition,      false);

  registerMooseObjectTask("add_function",                 Function,               false);
  registerMooseObjectTask("add_distribution",             Distribution,           false);
  registerMooseObjectTask("add_sampler",                  Sampler,                false);

  registerMooseObjectTask("add_aux_kernel",               AuxKernel,              false);
  appendMooseObjectTask  ("add_aux_kernel",               VectorAuxKernel);
  appendMooseObjectTask  ("add_aux_kernel",               ArrayAuxKernel);

  registerMooseObjectTask("add_scalar_kernel",            ScalarKernel,           false);
  registerMooseObjectTask("add_aux_scalar_kernel",        AuxScalarKernel,        false);
  registerMooseObjectTask("add_dirac_kernel",             DiracKernel,            false);
  appendMooseObjectTask  ("add_dirac_kernel",             VectorDiracKernel);
  registerMooseObjectTask("add_dg_kernel",                DGKernel,               false);
  registerMooseObjectTask("add_fv_kernel",                FVKernel,               false);
  registerMooseObjectTask("add_fv_bc",                    FVBoundaryCondition,    false);
  registerMooseObjectTask("add_fv_ik",                    FVInterfaceKernel,      false);
  registerMooseObjectTask("add_interface_kernel",         InterfaceKernel,        false);
  appendMooseObjectTask  ("add_interface_kernel",         VectorInterfaceKernel);
  registerMooseObjectTask("add_constraint",               Constraint,             false);

  registerMooseObjectTask("add_ic",                       InitialCondition,       false);
  appendMooseObjectTask  ("add_ic",                       ScalarInitialCondition);

  registerMooseObjectTask("add_damper",                   Damper,                 false);
  registerMooseObjectTask("setup_predictor",              Predictor,              false);
  registerMooseObjectTask("setup_time_stepper",           TimeStepper,            false);
  registerMooseObjectTask("setup_time_integrator",        TimeIntegrator,         false);

  registerMooseObjectTask("add_preconditioning",          MoosePreconditioner,    false);
  registerMooseObjectTask("add_field_split",              Split,                  false);

  registerMooseObjectTask("add_user_object",              UserObject,             false);
  appendMooseObjectTask  ("add_user_object",              Postprocessor);

  registerMooseObjectTask("add_postprocessor",            Postprocessor,          false);
  registerMooseObjectTask("add_vector_postprocessor",     VectorPostprocessor,    false);
  registerMooseObjectTask("add_reporter",                 Reporter,               false);

  registerMooseObjectTask("add_indicator",                Indicator,              false);
  registerMooseObjectTask("add_marker",                   Marker,                 false);

  registerMooseObjectTask("add_multi_app",                MultiApp,               false);
  registerMooseObjectTask("add_transfer",                 Transfer,               false);

  registerMooseObjectTask("add_output",                   Output,                 false);

  registerMooseObjectTask("add_control",                  Control,                false);
  registerMooseObjectTask("add_partitioner",              MoosePartitioner,       false);

  // clang-format on

  registerTask("dynamic_object_registration", false);
  registerTask("common_output", true);
  registerTask("setup_recover_file_base", true);
  registerTask("recover_meta_data", true);

  registerTask("add_bounds_vectors", false);
  registerTask("add_periodic_bc", false);
  registerTask("add_aux_variable", false);
  registerTask("add_external_aux_variables", true);
  registerTask("add_variable", false);
  registerTask("add_mortar_variable", false);

  registerTask("execute_mesh_generators", true);
  registerTask("uniform_refine_mesh", false);
  registerTask("prepare_mesh", false);
  registerTask("delete_remote_elements_after_late_geometric_ghosting", false);
  registerTask("setup_mesh_complete", true); // calls prepare
  registerTask("add_geometric_rm", false);
  registerTask("attach_geometric_rm", true);
  registerTask("attach_geometric_rm_final", true);

  registerTask("init_displaced_problem", false);

  registerTask("add_algebraic_rm", false);
  registerTask("attach_algebraic_rm", true);
  registerTask("add_coupling_rm", false);
  registerTask("attach_coupling_rm", true);
  registerTask("init_problem", true);
  registerTask("check_copy_nodal_vars", true);
  registerTask("copy_nodal_vars", true);
  registerTask("copy_nodal_aux_vars", true);
  registerTask("setup_postprocessor_data", false);

  registerTask("setup_dampers", true);
  registerTask("check_integrity", true);
  registerTask("resolve_optional_materials", true);
  registerTask("check_integrity_early", true);
  registerTask("setup_quadrature", true);

  /// Additional Actions
  registerTask("no_action", false); // Used for Empty Action placeholders
  registerTask("set_global_params", false);
  registerTask("setup_adaptivity", false);
  registerTask("meta_action", false);
  registerTask("setup_residual_debug", false);
  registerTask("setup_oversampling", false);
  registerTask("deprecated_block", false);
  registerTask("set_adaptivity_options", false);
  registerTask("add_mortar_interface", false);
  registerTask("coupling_functor_check", true);
  registerTask("add_master_action_material", false);

  // Dummy Actions (useful for sync points in the dependencies)
  registerTask("setup_function_complete", false);
  registerTask("setup_variable_complete", false);
  registerTask("ready_to_init", true);

  // Output related actions
  registerTask("add_output_aux_variables", true);
  registerTask("check_output", true);

  registerTask("create_problem_default", true);
  registerTask("create_problem_custom", false);
  registerTask("create_problem_complete", false);

  // Action for setting up the signal-based checkpoint
  registerTask("auto_checkpoint_action", true);
  /**************************/
  /****** Dependencies ******/
  /**************************/
  /**
   * The following is the default set of action dependencies for a basic MOOSE problem.  The
   * formatting of this string is important.  Each line represents a set of dependencies that depend
   * on the previous line.  Items on the same line have equal weight and can be executed in any
   * order.
   *
   * Additional dependencies can be inserted later inside of user applications with calls to
   * ActionWarehouse::addDependency("task", "pre_req")
   */

  // clang-format off
  syntax.addDependencySets("(meta_action)"
                           "(dynamic_object_registration)"
                           "(common_output)"
                           "(set_global_params)"
                           "(setup_recover_file_base)"
                           "(check_copy_nodal_vars)"
                           "(setup_mesh)"
                           "(add_geometric_rm)"
                           "(add_partitioner)"
                           "(add_mesh_generator)"
                           "(create_added_mesh_generators)"
                           "(append_mesh_generator)"
                           "(execute_mesh_generators)"
                           "(recover_meta_data)"
                           "(set_mesh_base)"
                           "(attach_geometric_rm)"
                           "(init_mesh)"
                           "(prepare_mesh)"
                           "(add_mortar_interface)"
                           "(uniform_refine_mesh)"
                           "(setup_mesh_complete)"
                           "(determine_system_type)"
                           "(create_problem)"
                           "(create_problem_custom)"
                           "(create_problem_default)"
                           "(create_problem_complete)"
                           "(setup_postprocessor_data)"
                           "(setup_time_integrator)"
                           "(setup_executioner)"
                           "(read_executor)"
                           "(add_executor)"
                           "(check_integrity_early)"
                           "(setup_predictor)"
                           "(init_displaced_problem)"
                           "(add_aux_variable, add_variable, add_elemental_field_variable,"
                           " add_external_aux_variables)"
                           "(add_mortar_variable)"
                           "(setup_variable_complete)"
                           "(setup_quadrature)"
                           "(add_function)"
                           "(add_periodic_bc)"
                           "(add_user_object)"
                           "(add_distribution)"
                           "(add_sampler)"
                           "(setup_function_complete)"
                           "(setup_adaptivity)"
                           "(set_adaptivity_options)"
                           "(add_ic)"
                           "(add_constraint, add_field_split)"
                           "(add_preconditioning)"
                           "(setup_time_stepper)"
                           "(ready_to_init)"
                           "(setup_dampers)"
                           "(setup_residual_debug)"
                           "(add_bounds_vectors)"
                           "(add_multi_app)"
                           "(add_transfer)"
                           "(copy_nodal_vars, copy_nodal_aux_vars)"
                           "(add_material)"
                           "(add_master_action_material)"
                           "(add_output_aux_variables)"
                           "(add_output)"
                           "(auto_checkpoint_action)"
                           "(add_postprocessor)"
                           "(add_vector_postprocessor)" // MaterialVectorPostprocessor requires this
                                                        // to be after material objects are created.
                           "(add_reporter)"
                           "(add_aux_kernel, add_bc, add_damper, add_dirac_kernel, add_kernel,"
                           " add_nodal_kernel, add_dg_kernel, add_fv_kernel, add_fv_bc, add_fv_ik,"
                           " add_interface_kernel, add_scalar_kernel, add_aux_scalar_kernel,"
                           " add_indicator, add_marker)"
                           "(resolve_optional_materials)"
                           "(add_algebraic_rm)"
                           "(add_coupling_rm)"
                           "(attach_geometric_rm_final)"
                           "(attach_algebraic_rm)"
                           "(attach_coupling_rm)"
                           "(coupling_functor_check)"
                           "(delete_remote_elements_after_late_geometric_ghosting)"
                           "(init_problem)"
                           "(add_control)"
                           "(check_output)"
                           "(check_integrity)");
  // clang-format on
}

/**
 * Multiple Action class can be associated with a single input file section, in which case all
 * associated Actions will be created and "acted" on when the associated input file section is
 * seen.*
 *
 * Example:
 *  "setup_mesh" <-----------> SetupMeshAction <---------
 *                                                        \
 *                                                         [Mesh]
 *                                                        /
 * "setup_mesh_complete" <---> SetupMeshCompleteAction <-
 *
 *
 * Action classes can also be registered to act on more than one input file section for a different
 * task if similar logic can work in multiple cases
 *
 * Example:
 * "add_variable" <-----                       -> [Variables/ *]
 *                       \                   /
 *                        CopyNodalVarsAction
 *                       /                   \
 * "add_aux_variable" <-                       -> [AuxVariables/ *]
 *
 *
 * Note: Placeholder "no_action" actions must be put in places where it is possible to match an
 *       object with a star or a more specific parent later on. (i.e. where one needs to negate the
 *       '*' matching prematurely).
 */
void
registerActions(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of registerActions");
  registerActions(syntax, action_factory, {"MooseApp"});
}

void
registerActions(Syntax & syntax,
                ActionFactory & action_factory,
                const std::set<std::string> & obj_labels)
{
  Registry::registerActionsTo(action_factory, obj_labels);

  // TODO: Why is this here?
  registerTask("finish_input_file_output", false);
}

void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  /**
   * Note: the optional third parameter is used to differentiate which task is
   * satisfied based on the syntax encountered for classes which are registered
   * to satisfy more than one task
   */
  registerSyntaxTask("CopyNodalVarsAction", "Variables/*", "check_copy_nodal_vars");
  registerSyntaxTask("CopyNodalVarsAction", "Variables/*", "copy_nodal_vars");
  registerSyntaxTask("CopyNodalVarsAction", "AuxVariables/*", "check_copy_nodal_vars");
  registerSyntaxTask("CopyNodalVarsAction", "AuxVariables/*", "copy_nodal_aux_vars");

  registerSyntaxTask("AddKernelAction", "Kernels/*", "add_kernel");
  registerSyntaxTask("AddNodalKernelAction", "NodalKernels/*", "add_nodal_kernel");
  registerSyntaxTask("AddKernelAction", "AuxKernels/*", "add_aux_kernel");
  registerSyntaxTask("AddKernelAction", "Bounds/*", "add_aux_kernel");

  registerSyntax("AddAuxKernelAction", "AuxVariables/*/AuxKernel");

  registerSyntaxTask("AddScalarKernelAction", "ScalarKernels/*", "add_scalar_kernel");
  registerSyntaxTask("AddScalarKernelAction", "AuxScalarKernels/*", "add_aux_scalar_kernel");

  registerSyntaxTask("AddBCAction", "BCs/*", "add_bc");

  registerSyntax("CreateProblemAction", "Problem");
  registerSyntax("DynamicObjectRegistrationAction", "Problem");

  registerSyntax("SetupMeshAction", "Mesh");
  registerSyntax("SetupMeshCompleteAction", "Mesh");
  registerSyntax("CreateDisplacedProblemAction", "Mesh");
  registerSyntax("DisplayGhostingAction", "Mesh");
  registerSyntax("AddMeshGeneratorAction", "Mesh/*");
  registerSyntax("ElementIDOutputAction", "Mesh");
  syntax.registerSyntaxType("Mesh/*", "MeshGeneratorName");

  registerSyntax("AddFunctionAction", "Functions/*");
  syntax.registerSyntaxType("Functions/*", "FunctionName");

  registerSyntax("GlobalParamsAction", "GlobalParams");

  registerSyntax("AddDistributionAction", "Distributions/*");
  syntax.registerSyntaxType("Distributions/*", "DistributionName");

  registerSyntax("AddSamplerAction", "Samplers/*");
  syntax.registerSyntaxType("Samplers/*", "SamplerName");

  registerSyntax("SetupDebugAction", "Debug");
  registerSyntax("SetupResidualDebugAction", "Debug");

  /// Variable/AuxVariable Actions
  registerSyntax("AddVariableAction", "Variables/*");
  syntax.registerSyntaxType("Variables/*", "VariableName");
  syntax.registerSyntaxType("Variables/*", "NonlinearVariableName");

  registerSyntax("AddICAction", "Variables/*/InitialCondition");

  registerSyntax("AddAuxVariableAction", "AuxVariables/*");
  syntax.registerSyntaxType("AuxVariables/*", "VariableName");
  syntax.registerSyntaxType("AuxVariables/*", "AuxVariableName");

  registerSyntax("AddICAction", "AuxVariables/*/InitialCondition");

  registerSyntaxTask("EmptyAction", "BCs/Periodic", "no_action"); // placeholder
  registerSyntax("AddPeriodicBCAction", "BCs/Periodic/*");

  registerSyntaxTask("AddInitialConditionAction", "ICs/*", "add_ic");

  registerSyntax("AddMaterialAction", "Materials/*");
  syntax.registerSyntaxType("Materials/*", "MaterialName");

  registerSyntax("AddPostprocessorAction", "Postprocessors/*");
  syntax.registerSyntaxType("Postprocessors/*", "PostprocessorName");
  syntax.registerSyntaxType("Postprocessors/*", "UserObjectName");

  registerSyntax("AddVectorPostprocessorAction", "VectorPostprocessors/*");
  syntax.registerSyntaxType("VectorPostprocessors/*", "VectorPostprocessorName");

  registerSyntax("AddReporterAction", "Reporters/*");
  syntax.registerSyntaxType("Reporters/*", "ReporterName");

  registerSyntax("AddDamperAction", "Dampers/*");

  registerSyntax("AddOutputAction", "Outputs/*");
  registerSyntax("CommonOutputAction", "Outputs");
  syntax.registerSyntaxType("Outputs/*", "OutputName");

  // Note: Preconditioner Actions will be built by this setup action
  registerSyntax("SetupPreconditionerAction", "Preconditioning/*");
  registerSyntax("AddFieldSplitAction", "Preconditioning/*/*");

  registerSyntax("CreateExecutionerAction", "Executioner");
  registerSyntax("ReadExecutorParamsAction", "Executors/*");
  registerSyntax("SetupTimeStepperAction", "Executioner/TimeStepper");
  registerSyntax("SetupTimeIntegratorAction", "Executioner/TimeIntegrator");
  syntax.registerSyntaxType("Executors/*", "ExecutorName");

  registerSyntax("SetupQuadratureAction", "Executioner/Quadrature");
  registerSyntax("SetupPredictorAction", "Executioner/Predictor");
#ifdef LIBMESH_ENABLE_AMR
  registerSyntax("AdaptivityAction", "Executioner/Adaptivity");
#endif

  registerSyntax("PartitionerAction", "Mesh/Partitioner");

  registerSyntax("AddDiracKernelAction", "DiracKernels/*");

  registerSyntax("AddDGKernelAction", "DGKernels/*");
  registerSyntax("AddFVKernelAction", "FVKernels/*");
  registerSyntax("AddFVBCAction", "FVBCs/*");
  registerSyntax("AddFVInterfaceKernelAction", "FVInterfaceKernels/*");
  registerSyntax("CheckFVBCAction", "FVBCs");

  registerSyntax("AddInterfaceKernelAction", "InterfaceKernels/*");

  registerSyntax("AddConstraintAction", "Constraints/*");

  registerSyntax("AddUserObjectAction", "UserObjects/*");
  syntax.registerSyntaxType("UserObjects/*", "UserObjectName");
  registerSyntax("AddControlAction", "Controls/*");
  registerSyntax("AddBoundsVectorsAction", "Bounds");

  registerSyntax("AddNodalNormalsAction", "NodalNormals");

  // Indicator
  registerSyntax("AddElementalFieldAction", "Adaptivity/Indicators/*");
  registerSyntax("AddIndicatorAction", "Adaptivity/Indicators/*");
  syntax.registerSyntaxType("Adaptivity/Indicators/*", "IndicatorName");

  // Marker
  registerSyntax("AddElementalFieldAction", "Adaptivity/Markers/*");
  registerSyntax("AddMarkerAction", "Adaptivity/Markers/*");
  syntax.registerSyntaxType("Adaptivity/Markers/*", "MarkerName");

  // New Adaptivity System
  registerSyntax("SetAdaptivityOptionsAction", "Adaptivity");

  // Deprecated Block
  registerSyntax("DeprecatedBlockAction", "DeprecatedBlock");

  // Multi Apps
  registerSyntax("AddMultiAppAction", "MultiApps/*");
  syntax.registerSyntaxType("MultiApps/*", "MultiAppName");

  // Transfers
  registerSyntax("AddTransferAction", "Transfers/*");

  // Material derivative test
  registerSyntaxTask("EmptyAction", "Debug/MaterialDerivativeTest", "no_action"); // placeholder
  registerSyntax("MaterialDerivativeTestAction", "Debug/MaterialDerivativeTest/*");

  addActionTypes(syntax);
}

void
associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  associateSyntaxInner(syntax, action_factory);
  registerActions(syntax, action_factory);
}

void
setSolverDefaults(FEProblemBase & problem)
{
  // May be a touch expensive to create a new DM every time, but probably safer to do it this way
  Moose::PetscSupport::petscSetDefaults(problem);
}

MPI_Comm
swapLibMeshComm(MPI_Comm new_comm)
{
  MPI_Comm old_comm = PETSC_COMM_WORLD;
  PETSC_COMM_WORLD = new_comm;
  return old_comm;
}

static bool _color_console = isatty(fileno(stdout));

bool
colorConsole()
{
  return _color_console;
}

bool
setColorConsole(bool use_color, bool force)
{
  _color_console = (isatty(fileno(stdout)) || force) && use_color;
  return _color_console;
}

bool _warnings_are_errors = false;
bool _deprecated_is_error = false;
bool _throw_on_error = false;
bool _throw_on_warning = false;
int interrupt_signal_number = 0;
bool show_trace = true;
bool show_multiple = false;

} // namespace Moose
