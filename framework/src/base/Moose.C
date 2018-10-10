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

#include <unistd.h>

// Define the available execute flags for MOOSE. The flags using a hex value are setup to retain the
// same numbers that were utilized with older versions of MOOSE for keeping existing applications
// working using the deprecated flags. In the future, as in the EXEC_SAME_AS_MULTIAPP flag, there is
// no reason to keep these flags bitwise comparable or to assigned an id because the MultiMooseEnum
// that is used to store these has convenience methods for determining the what flags are active.
const ExecFlagType EXEC_NONE("NONE", 0x00);                     // 0
const ExecFlagType EXEC_INITIAL("INITIAL", 0x01);               // 1
const ExecFlagType EXEC_LINEAR("LINEAR", 0x02);                 // 2
const ExecFlagType EXEC_NONLINEAR("NONLINEAR", 0x04);           // 4
const ExecFlagType EXEC_TIMESTEP_END("TIMESTEP_END", 0x08);     // 8
const ExecFlagType EXEC_TIMESTEP_BEGIN("TIMESTEP_BEGIN", 0x10); // 16
const ExecFlagType EXEC_FINAL("FINAL", 0x20);                   // 32
const ExecFlagType EXEC_FORCED("FORCED", 0x40);                 // 64
const ExecFlagType EXEC_FAILED("FAILED", 0x80);                 // 128
const ExecFlagType EXEC_CUSTOM("CUSTOM", 0x100);                // 256
const ExecFlagType EXEC_SUBDOMAIN("SUBDOMAIN", 0x200);          // 512
const ExecFlagType EXEC_SAME_AS_MULTIAPP("SAME_AS_MULTIAPP");

namespace Moose
{

void associateSyntaxInner(Syntax & syntax, ActionFactory & action_factory);

void
registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  registerObjects(f, {"MooseApp"});
  associateSyntaxInner(s, af);
  registerActions(s, af, {"MooseApp"});
  registerExecFlags(f);
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

  // This task does not construct an object, but it needs all of the parameters that
  // would normally be used to construct an object.
  registerMooseObjectTask("determine_system_type",        Executioner,            true);

  registerMooseObjectTask("setup_mesh",                   MooseMesh,              false);
  registerMooseObjectTask("init_mesh",                    MooseMesh,              false);
  registerMooseObjectTask("add_mesh_modifier",            MeshModifier,           false);

  registerMooseObjectTask("add_kernel",                   Kernel,                 false);
  appendMooseObjectTask  ("add_kernel",                   EigenKernel);
  appendMooseObjectTask  ("add_kernel",                   VectorKernel);

  registerMooseObjectTask("add_nodal_kernel",             NodalKernel,            false);

  registerMooseObjectTask("add_material",                 Material,               false);
  registerMooseObjectTask("add_bc",                       BoundaryCondition,      false);
  registerMooseObjectTask("add_function",                 Function,               false);
  registerMooseObjectTask("add_distribution",             Distribution,           false);
  registerMooseObjectTask("add_sampler",                  Sampler,                false);

  registerMooseObjectTask("add_aux_kernel",               AuxKernel,              false);
  registerMooseObjectTask("add_elemental_field_variable", AuxKernel,              false);

  registerMooseObjectTask("add_scalar_kernel",            ScalarKernel,           false);
  registerMooseObjectTask("add_aux_scalar_kernel",        AuxScalarKernel,        false);
  registerMooseObjectTask("add_dirac_kernel",             DiracKernel,            false);
  registerMooseObjectTask("add_dg_kernel",                DGKernel,               false);
  registerMooseObjectTask("add_interface_kernel",         InterfaceKernel,        false);
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

  registerTask("add_bounds_vectors", false);
  registerTask("add_periodic_bc", false);
  registerTask("add_aux_variable", false);
  registerTask("add_external_aux_variables", true);
  registerTask("add_variable", false);

  registerTask("execute_mesh_modifiers", false);
  registerTask("uniform_refine_mesh", false);
  registerTask("prepare_mesh", false);
  registerTask("add_geometric_rm", true);
  registerTask("setup_mesh_complete", false); // calls prepare

  registerTask("init_displaced_problem", false);

  registerTask("add_algebraic_rm", true);
  registerTask("init_problem", true);
  registerTask("check_copy_nodal_vars", true);
  registerTask("copy_nodal_vars", true);
  registerTask("copy_nodal_aux_vars", true);
  registerTask("setup_postprocessor_data", false);

  registerTask("setup_dampers", true);
  registerTask("check_integrity", true);
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

  // Deprecated tasks
  registerTask("setup_material_output", false);
  registerTask("setup_debug", false);

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
  syntax.addDependencySets("(meta_action)"
                           "(dynamic_object_registration)"
                           "(common_output)"
                           "(set_global_params)"
                           "(setup_recover_file_base)"
                           "(check_copy_nodal_vars)"
                           "(setup_mesh)"
                           "(add_partitioner)"
                           "(add_geometric_rm)"
                           "(init_mesh)"
                           "(prepare_mesh)"
                           "(add_mesh_modifier)"
                           "(execute_mesh_modifiers)"
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
                           "(check_integrity_early)"
                           "(setup_predictor)"
                           "(init_displaced_problem)"
                           "(add_aux_variable, add_variable, add_elemental_field_variable,"
                           " add_external_aux_variables)"
                           "(setup_variable_complete)"
                           "(setup_quadrature)"
                           "(add_function)"
                           "(add_distribution)"
                           "(add_sampler)"
                           "(add_periodic_bc)"
                           "(add_user_object)"
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
                           "(setup_material_output)" // DEPRECATED: Remove by 12/31/2018
                           "(add_output_aux_variables)"
                           "(add_algebraic_rm)"
                           "(init_problem)"
                           "(setup_debug)" // DEPRECATED: Remove by 12/31/2018
                           "(add_output)"
                           "(add_postprocessor)"
                           "(add_vector_postprocessor)" // MaterialVectorPostprocessor requires this
                                                        // to be after material objects are created.
                           "(add_aux_kernel, add_bc, add_damper, add_dirac_kernel, add_kernel,"
                           " add_nodal_kernel, add_dg_kernel, add_interface_kernel,"
                           " add_scalar_kernel, add_aux_scalar_kernel, add_indicator, add_marker)"
                           "(add_control)"
                           "(check_output)"
                           "(check_integrity)");
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
registerExecFlags(Factory & factory)
{
  registerExecFlag(EXEC_NONE);
  registerExecFlag(EXEC_INITIAL);
  registerExecFlag(EXEC_LINEAR);
  registerExecFlag(EXEC_NONLINEAR);
  registerExecFlag(EXEC_TIMESTEP_END);
  registerExecFlag(EXEC_TIMESTEP_BEGIN);
  registerExecFlag(EXEC_FINAL);
  registerExecFlag(EXEC_FORCED);
  registerExecFlag(EXEC_FAILED);
  registerExecFlag(EXEC_CUSTOM);
  registerExecFlag(EXEC_SUBDOMAIN);
  registerExecFlag(EXEC_SAME_AS_MULTIAPP);
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

  registerSyntaxTask("AddScalarKernelAction", "ScalarKernels/*", "add_scalar_kernel");
  registerSyntaxTask("AddScalarKernelAction", "AuxScalarKernels/*", "add_aux_scalar_kernel");

  registerSyntaxTask("AddBCAction", "BCs/*", "add_bc");

  registerSyntax("CreateProblemAction", "Problem");
  registerSyntax("DynamicObjectRegistrationAction", "Problem");
  registerSyntax("SetupMeshAction", "Mesh");
  registerSyntax("SetupMeshCompleteAction", "Mesh");
  //  registerSyntaxTask("SetupMeshCompleteAction", "Mesh", "prepare_mesh");
  //  registerSyntaxTask("SetupMeshCompleteAction", "Mesh", "setup_mesh_complete");
  registerSyntax("CreateDisplacedProblemAction", "Mesh");
  registerSyntax("AddMeshModifierAction", "MeshModifiers/*");
  registerSyntax("AddMortarInterfaceAction", "Mesh/MortarInterfaces/*");

  registerSyntax("AddFunctionAction", "Functions/*");
  syntax.registerSyntaxType("Functions/*", "FunctionName");

  registerSyntax("GlobalParamsAction", "GlobalParams");

  registerSyntax("AddDistributionAction", "Distributions/*");
  registerSyntax("AddSamplerAction", "Samplers/*");

  registerSyntax("SetupDebugAction", "Debug");
  registerSyntax("SetupResidualDebugAction", "Debug");

  /// Variable/AuxVariable Actions
  registerSyntax("AddVariableAction", "Variables/*");
  syntax.registerSyntaxType("Variables/*", "VariableName");
  syntax.registerSyntaxType("Variables/*", "NonlinearVariableName");
  //  syntax.registerActionSyntax("AddVariableAction", "Variables/*", "add_variable");
  //  syntax.registerActionSyntax("AddVariableAction", "Variables/*", "add_ic");

  registerSyntax("AddICAction", "Variables/*/InitialCondition");

  registerSyntax("AddAuxVariableAction", "AuxVariables/*");
  syntax.registerSyntaxType("AuxVariables/*", "VariableName");
  syntax.registerSyntaxType("AuxVariables/*", "AuxVariableName");
  //  syntax.registerActionSyntax("AddAuxVariableAction", "AuxVariables/*", "add_aux_variable");
  //  syntax.registerActionSyntax("AddAuxVariableAction", "AuxVariables/*", "add_ic");

  registerSyntax("AddICAction", "AuxVariables/*/InitialCondition");

  registerSyntaxTask("EmptyAction", "BCs/Periodic", "no_action"); // placeholder
  registerSyntax("AddPeriodicBCAction", "BCs/Periodic/*");

  registerSyntaxTask("AddInitialConditionAction", "ICs/*", "add_ic");

  registerSyntax("AddMaterialAction", "Materials/*");

  registerSyntax("SetupPostprocessorDataAction", "Postprocessors/*");
  registerSyntax("AddPostprocessorAction", "Postprocessors/*");
  syntax.registerSyntaxType("Postprocessors/*", "PostprocessorName");
  syntax.registerSyntaxType("Postprocessors/*", "UserObjectName");

  registerSyntax("AddVectorPostprocessorAction", "VectorPostprocessors/*");
  syntax.registerSyntaxType("VectorPostprocessors/*", "VectorPostprocessorName");

  registerSyntax("AddDamperAction", "Dampers/*");

  registerSyntax("AddOutputAction", "Outputs/*");
  registerSyntax("CommonOutputAction", "Outputs");
  syntax.registerSyntaxType("Outputs/*", "OutputName");

  // Note: Preconditioner Actions will be built by this setup action
  registerSyntax("SetupPreconditionerAction", "Preconditioning/*");
  registerSyntax("AddFieldSplitAction", "Preconditioning/*/*");

  registerSyntax("CreateExecutionerAction", "Executioner");
  registerSyntax("SetupTimeStepperAction", "Executioner/TimeStepper");
  registerSyntax("SetupTimeIntegratorAction", "Executioner/TimeIntegrator");

  registerSyntax("SetupQuadratureAction", "Executioner/Quadrature");
  registerSyntax("SetupPredictorAction", "Executioner/Predictor");
#ifdef LIBMESH_ENABLE_AMR
  registerSyntax("AdaptivityAction", "Executioner/Adaptivity");
#endif

  registerSyntax("PartitionerAction", "Mesh/Partitioner");

  registerSyntax("AddDiracKernelAction", "DiracKernels/*");

  registerSyntax("AddDGKernelAction", "DGKernels/*");

  registerSyntax("AddInterfaceKernelAction", "InterfaceKernels/*");

  registerSyntax("AddConstraintAction", "Constraints/*");

  registerSyntax("AddUserObjectAction", "UserObjects/*");
  syntax.registerSyntaxType("UserObjects/*", "UserObjectName");
  registerSyntax("AddControlAction", "Controls/*");
  registerSyntax("AddBoundsVectorsAction", "Bounds");

  registerSyntax("AddNodalNormalsAction", "NodalNormals");
  //  registerSyntaxTask("AddNodalNormalsAction", "NodalNormals", "add_aux_variable");
  //  registerSyntaxTask("AddNodalNormalsAction", "NodalNormals", "add_postprocessor");
  //  registerSyntaxTask("AddNodalNormalsAction", "NodalNormals", "add_user_object");

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
#ifdef LIBMESH_HAVE_PETSC
  // May be a touch expensive to create a new DM every time, but probably safer to do it this way
  Moose::PetscSupport::petscSetDefaults(problem);
#endif // LIBMESH_HAVE_PETSC
}

MPI_Comm
swapLibMeshComm(MPI_Comm new_comm)
{
#ifdef LIBMESH_HAVE_PETSC
  MPI_Comm old_comm = PETSC_COMM_WORLD;
  PETSC_COMM_WORLD = new_comm;
  return old_comm;
#endif // LIBMESH_HAVE_PETSC
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

} // namespace Moose
