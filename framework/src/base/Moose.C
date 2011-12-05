/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Moose.h"
#include "Factory.h"
#include "ProblemFactory.h"
#include "NonlinearSystem.h"
#include "PetscSupport.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"

// objects that can be created by MOOSE
// problems
#include "FEProblem.h"
#include "OutputProblem.h"
#include "CoupledProblem.h"
// kernels
#include "TimeDerivative.h"
#include "Diffusion.h"
#include "CoupledForce.h"
#include "UserForcingFunction.h"
#include "BodyForce.h"
#include "ImplicitEuler.h"
#include "Reaction.h"
#include "RealPropertyOutput.h"

// bcs
#include "ConvectiveFluxBC.h"
#include "DirichletBC.h"
#include "PresetBC.h"
#include "NeumannBC.h"
#include "FunctionDirichletBC.h"
#include "FunctionPresetBC.h"
#include "FunctionNeumannBC.h"
#include "MatchedValueBC.h"
#include "VacuumBC.h"
#include "DashpotBC.h"
#include "DirichletPostprocessorBC.h"
#include "SinDirichletBC.h"
#include "SinNeumannBC.h"
#include "VectorNeumannBC.h"
#include "WeakGradientBC.h"
// auxkernels
#include "CoupledAux.h"
#include "ConstantAux.h"
#include "FunctionAux.h"
#include "NearestNodeDistanceAux.h"
#include "NearestNodeValueAux.h"
#include "PenetrationAux.h"
#include "ProcessorIDAux.h"
#include "SelfAux.h"
#include "GapValueAux.h"
#include "MaterialRealAux.h"
#include "DebugResidualAux.h"
// dirac kernels
#include "ConstantPointSource.h"

// DG kernels
#include "DGDiffusion.h"

// ics
#include "ConstantIC.h"
#include "BoundingBoxIC.h"
#include "FunctionIC.h"
#include "RandomIC.h"

// mesh modifiers
#include "ElementDeleter.h"

// executioners
#include "Steady.h"
#include "Transient.h"
#include "LooseCoupling.h"
#include "SolutionTimeAdaptive.h"
#include "DT2Transient.h"

// functions
#include "Composite.h"
#include "ParsedFunction.h"
#include "ParsedGradFunction.h"
#include "PiecewiseLinear.h"
#include "SolutionFunction.h"
#include "SphereFunction.h"
#include "PiecewiseBilinear.h"
#include "PiecewiseLinearFile.h"

// materials
#include "GenericConstantMaterial.h"

// PPS
#include "AverageElementSize.h"
#include "AverageNodalVariableValue.h"
#include "ElementAverageValue.h"
#include "ElementH1Error.h"
#include "ElementH1SemiError.h"
#include "ElementIntegral.h"
#include "ElementL2Error.h"
#include "EmptyPostprocessor.h"
#include "NodalVariableValue.h"
#include "PrintDOFs.h"
#include "PrintDT.h"
#include "PrintElapsedTime.h"
#include "PrintNumElems.h"
#include "PrintNumNodes.h"
#include "PrintNumNonlinearIters.h"
#include "PrintNumLinearIters.h"
#include "PrintResidual.h"
#include "Reporter.h"
#include "SideAverageValue.h"
#include "SideFluxIntegral.h"
#include "SideIntegral.h"
#include "NodalMaxValue.h"

// dampers
#include "ConstantDamper.h"
#include "MaxIncrement.h"

// DG
#include "DGDiffusion.h"

// Constraints
#include "TiedValueConstraint.h"

// Actions
#include "AddMeshModifierAction.h"
#include "AddBCAction.h"
#include "AddDiracKernelAction.h"
#include "AddICAction.h"
#include "AddKernelAction.h"
#include "AddDGKernelAction.h"
#include "AddPeriodicBCAction.h"
#include "AddVariableAction.h"
#include "AddPostprocessorAction.h"
#include "AddDamperAction.h"
#include "AddFunctionAction.h"
#include "CreateExecutionerAction.h"
#include "CreateMeshAction.h"
#include "ReadMeshAction.h"
#include "EmptyAction.h"
#include "InitProblemAction.h"
#include "CopyNodalVarsAction.h"
#include "SetupMeshAction.h"
#include "AddExtraNodesetAction.h"
#include "SetupOutputAction.h"
#include "AddMaterialAction.h"
#include "GlobalParamsAction.h"
#include "AdaptivityAction.h"
#include "SetupDampersAction.h"
#include "CheckIntegrityAction.h"
#include "SetupQuadratureAction.h"
#include "SetupPreconditionerAction.h"
#include "SetupPBPAction.h"
#include "SetupSMPAction.h"
#include "SetupFiniteDifferencePreconditionerAction.h"
#include "SetupDebugAction.h"
#include "SetupResidualDebugAction.h"
#include "InitialRefinementAction.h"
#include "SetupOverSamplingAction.h"
#include "DeprecatedBlockAction.h"
#include "AddConstraintAction.h"
#include "InitDisplacedProblemAction.h"

namespace Moose {

static bool registered = false;

void
registerObjects()
{
  if (registered)
    return;

  // problems
  registerProblem(FEProblem);
  registerProblem(OutputProblem);
  registerProblem(CoupledProblem);

  // kernels
  registerKernel(TimeDerivative);
  registerKernel(Diffusion);
  registerKernel(CoupledForce);
  registerKernel(UserForcingFunction);
  registerKernel(BodyForce);
  registerKernel(ImplicitEuler);
  registerKernel(Reaction);
  registerKernel(RealPropertyOutput);

  // bcs
  registerBoundaryCondition(ConvectiveFluxBC);
  registerBoundaryCondition(DirichletBC);
  registerBoundaryCondition(PresetBC);
  registerBoundaryCondition(NeumannBC);
  registerBoundaryCondition(FunctionDirichletBC);
  registerBoundaryCondition(FunctionPresetBC);
  registerBoundaryCondition(FunctionNeumannBC);
  registerBoundaryCondition(MatchedValueBC);
  registerBoundaryCondition(VacuumBC);

  registerBoundaryCondition(DashpotBC);
  registerBoundaryCondition(DirichletPostprocessorBC);
  registerBoundaryCondition(SinDirichletBC);
  registerBoundaryCondition(SinNeumannBC);
  registerBoundaryCondition(VectorNeumannBC);
  registerBoundaryCondition(WeakGradientBC);

  // dirac kernels
  registerDiracKernel(ConstantPointSource);

  // aux kernels
  registerAux(CoupledAux);
  registerAux(ConstantAux);
  registerAux(FunctionAux);
  registerAux(NearestNodeDistanceAux);
  registerAux(NearestNodeValueAux);
  registerAux(PenetrationAux);
  registerAux(ProcessorIDAux);
  registerAux(SelfAux);
  registerAux(GapValueAux);
  registerAux(MaterialRealAux);
  registerAux(DebugResidualAux);

  // Initial Conditions
  registerInitialCondition(ConstantIC);
  registerInitialCondition(BoundingBoxIC);
  registerInitialCondition(FunctionIC);
  registerInitialCondition(RandomIC);
  // Mesh Modifiers
  registerMeshModifier(ElementDeleter);
  // executioners
  registerExecutioner(Steady);
  registerExecutioner(Transient);
  registerExecutioner(LooseCoupling);
  registerExecutioner(SolutionTimeAdaptive);
  registerExecutioner(DT2Transient);
  // functions
  registerFunction(Composite);
  registerFunction(ParsedFunction);
  registerFunction(ParsedGradFunction);
  registerFunction(PiecewiseLinear);
  registerFunction(SolutionFunction);
  registerFunction(SphereFunction);
  registerFunction(PiecewiseBilinear);
  registerFunction(PiecewiseLinearFile);
  // materials
  registerMaterial(GenericConstantMaterial);
  // PPS
  registerPostprocessor(AverageElementSize);
  registerPostprocessor(AverageNodalVariableValue);
  registerPostprocessor(ElementAverageValue);
  registerPostprocessor(ElementH1Error);
  registerPostprocessor(ElementH1SemiError);
  registerPostprocessor(ElementIntegral);
  registerPostprocessor(ElementL2Error);
  registerPostprocessor(EmptyPostprocessor);
  registerPostprocessor(NodalVariableValue);
  registerPostprocessor(PrintDOFs);
  registerPostprocessor(PrintDT);
  registerPostprocessor(PrintElapsedTime);
  registerPostprocessor(PrintNumElems);
  registerPostprocessor(PrintNumNodes);
  registerPostprocessor(PrintNumNonlinearIters);
  registerPostprocessor(PrintNumLinearIters);
  registerPostprocessor(PrintResidual);
  registerPostprocessor(Reporter);
  registerPostprocessor(SideAverageValue);
  registerPostprocessor(SideFluxIntegral);
  registerPostprocessor(SideIntegral);
  registerPostprocessor(NodalMaxValue);
  // dampers
  registerDamper(ConstantDamper);
  registerDamper(MaxIncrement);
  // DG
  registerDGKernel(DGDiffusion);

  // Constraints
  registerConstraint(TiedValueConstraint);

  addActionTypes();
  registerActions();

  registered = true;
}

void
addActionTypes()
{
  /**************************/
  /**** Register Actions ****/
  /**************************/
  /// Minimal Problem
  registerActionName("setup_mesh", true);
  registerActionName("add_variable", true);
  registerActionName("add_kernel", true);
  registerActionName("setup_executioner", true);
  registerActionName("init_displaced_problem", false);
  registerActionName("setup_output", true);
  registerActionName("init_problem", true);
  registerActionName("copy_nodal_vars", true);
  registerActionName("copy_nodal_aux_vars", true);
  registerActionName("add_bc", false);  // Does this need to be true?  Not if you have periodic boundaries...
  registerActionName("setup_dampers", true);
  registerActionName("check_integrity", true);
  registerActionName("setup_quadrature", true);

  /// Additional Actions
  registerActionName("no_action", false);  // Used for Empty Action placeholders
  registerActionName("set_global_params", false);
  registerActionName("create_mesh", false);
  registerActionName("read_mesh", false);
  registerActionName("add_mesh_modifier", false);
  registerActionName("add_extra_nodeset", false);
  registerActionName("add_material", false);
  registerActionName("add_function", false);
  registerActionName("add_aux_variable", false);
  registerActionName("add_aux_kernel", false);
  registerActionName("add_aux_bc", false);
  registerActionName("add_dirac_kernel", false);
  registerActionName("add_dg_kernel", false);
  registerActionName("add_ic", false);
  registerActionName("add_postprocessor", false);
  registerActionName("add_damper", false);
  registerActionName("add_periodic_bc", false);
  registerActionName("preconditioning_meta_action", false);
  registerActionName("add_preconditioning", false);
  registerActionName("setup_adaptivity", false);
  registerActionName("meta_action", false);
  registerActionName("initial_mesh_refinement", false);
  registerActionName("setup_debug", false);
  registerActionName("setup_residual_debug", false);
  registerActionName("setup_oversampling", false);
  registerActionName("deprecated_block", false);
  registerActionName("add_constraint", false);

  // Dummy Actions (useful for sync points in the dependencies)
  registerActionName("setup_mesh_complete", false);
  registerActionName("setup_function_complete", false);
  registerActionName("setup_variable_complete", false);
  registerActionName("ready_to_init", true);
  registerActionName("setup_pps_complete", false);

  /**************************/
  /****** Dependencies ******/
  /**************************/
  /**
   * The following is the default set of action dependencies for a basic MOOSE problem.  The formatting
   * of this string is important.  Each line represents a set of dependencies that depend on the previous
   * line.  Items on the same line have equal weight and can be executed in any order.
   *
   * Additional dependencies can be inserted later inside of user applications with calls to
   * ActionWarehouse::addDependency("action_name", "pre_req")
   */
  action_warehouse.addDependencySets(
"(meta_action)"
"(set_global_params)"
"(create_mesh, read_mesh)"
"(add_extra_nodeset)"
"(setup_mesh)"
"(add_mesh_modifier, setup_mesh_complete)"
"(setup_executioner)"
"(init_displaced_problem)"
"(add_function)"
"(setup_function_complete)"
"(add_aux_variable, add_variable)"
"(setup_variable_complete)"
"(setup_adaptivity)"
"(add_ic, add_periodic_bc)"
"(preconditioning_meta_action)"
"(add_preconditioning, add_constraint)"
"(ready_to_init)"
"(setup_quadrature)"
"(setup_dampers)"
"(setup_residual_debug)"
"(init_problem)"
"(copy_nodal_vars, copy_nodal_aux_vars)"
"(initial_mesh_refinement)"
"(add_material)"
"(add_postprocessor)"
"(setup_pps_complete)"
"(add_aux_bc, add_aux_kernel, add_bc, add_damper, add_dirac_kernel, add_kernel, add_dg_kernel, setup_output)"
"(setup_oversampling)"
"(check_integrity)"
"(setup_debug)"
);

}

/**
 * Multiple Action class can be associated with a single input file section, in which case all associated Actions
 * will be created and "acted" on when the associated inputfile section is seen.
 *
 * Example:
 * "setup_mesh" <---> SetupMeshAction <-
 *                                       \
 *                                        [Mesh]
 *                                       /
 * "read_mesh"  <---> ReadMeshAction  <-
 *
 *
 * Action classes can also be registered to act on more than one input file section for a different action_name
 * if similar logic can work in multiple cases
 *
 * Example:
 * "add_variable" <-----                       -> [Variables/ *]
 *                       \                   /
 *                         AddVariableAction
 *                       /                   \
 * "add_aux_variable" <-                       -> [AuxVariables/ *]
 *
 *
 * Note: Placeholder "no_action" actions must be put in places where it is possible to match an object
 *       with a star or a more specific parent later on. (i.e. where one needs to negate the '*' matching
 *       prematurely)
 */
void
registerActions()
{
  registerAction(CreateMeshAction, "create_mesh");
  registerAction(ReadMeshAction, "read_mesh");
  registerAction(SetupMeshAction, "setup_mesh");
  registerAction(AddExtraNodesetAction, "add_extra_nodeset");

  registerAction(AddFunctionAction, "add_function");
  registerAction(CreateExecutionerAction, "setup_executioner");
  registerAction(InitDisplacedProblemAction, "init_displaced_problem");
  registerAction(SetupOutputAction, "setup_output");
  registerAction(GlobalParamsAction, "set_global_params");

  /// MooseObjectActions
  registerAction(AddMeshModifierAction, "add_mesh_modifier");

  /// Variable/AuxVariable Actions
  registerAction(AddVariableAction, "add_variable");
  registerAction(CopyNodalVarsAction, "copy_nodal_vars");
  registerAction(AddVariableAction, "add_aux_variable");
  registerAction(CopyNodalVarsAction, "copy_nodal_aux_vars");

  registerAction(AddICAction, "add_ic");
  registerAction(AddKernelAction, "add_kernel");
  registerAction(AddKernelAction, "add_aux_kernel");
  registerAction(AddDGKernelAction, "add_dg_kernel");
  registerAction(AddBCAction, "add_bc");
  registerAction(EmptyAction, "no_action");  // placeholder
  registerAction(AddPeriodicBCAction, "add_periodic_bc");
  registerAction(AddBCAction, "add_aux_bc");
  registerAction(AddMaterialAction, "add_material");
  registerAction(AddPostprocessorAction, "add_postprocessor");
  registerAction(AddDamperAction, "add_damper");
  registerAction(SetupPreconditionerAction, "preconditioning_meta_action");
  registerAction(SetupPBPAction, "add_preconditioning");
  registerAction(SetupSMPAction, "add_preconditioning");
  registerAction(SetupFiniteDifferencePreconditionerAction, "add_preconditioning");
  registerAction(SetupQuadratureAction, "setup_quadrature");
  registerAction(SetupOverSamplingAction, "setup_oversampling");
  registerAction(DeprecatedBlockAction, "deprecated_block");
  registerAction(AddConstraintAction, "add_constraint");

#ifdef LIBMESH_ENABLE_AMR
  registerAction(AdaptivityAction, "setup_adaptivity");
  registerAction(InitialRefinementAction, "initial_mesh_refinement");
#endif

  registerAction(AddDiracKernelAction, "add_dirac_kernel");
  registerAction(SetupDebugAction, "setup_debug");
  registerAction(SetupResidualDebugAction, "setup_residual_debug");

  // NonParsedActions
  registerAction(SetupDampersAction, "setup_dampers");
  registerAction(EmptyAction, "ready_to_init");
  registerAction(InitProblemAction, "init_problem");
  registerAction(CheckIntegrityAction, "check_integrity");

  // TODO: Why is this here?
  registerActionName("finish_input_file_output", false);
  registerAction(EmptyAction, "finish_input_file_output");
}

void
setSolverDefaults(FEProblem & problem)
{
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetDefaults(problem);
#endif //LIBMESH_HAVE_PETSC
}

// TODO: Move this (perhaps into the problem_warehouse?)
ActionWarehouse action_warehouse;


PerfLog setup_perf_log("Setup");

Executioner *executioner = NULL;

} // namespace Moose
