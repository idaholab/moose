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
#include "NonlinearSystem.h"
#include "PetscSupport.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "Syntax.h"

// objects that can be created by MOOSE
#include "MooseMesh.h"
#include "GeneratedMesh.h"
#include "MeshExtruder.h"
// problems
#include "FEProblem.h"
#include "OutputProblem.h"
#include "CoupledProblem.h"
// kernels
#include "TimeDerivative.h"
#include "Diffusion.h"
#include "AnisotropicDiffusion.h"
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
#include "SinDirichletBC.h"
#include "SinNeumannBC.h"
#include "VectorNeumannBC.h"
#include "WeakGradientBC.h"
// auxkernels
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
#include "BoundsAux.h"
#include "LayeredIntegralAux.h"
#include "LayeredSideIntegralAux.h"

// dirac kernels
#include "ConstantPointSource.h"

// DG kernels
#include "DGDiffusion.h"

// ics
#include "ConstantIC.h"
#include "BoundingBoxIC.h"
#include "FunctionIC.h"
#include "RandomIC.h"
#include "ScalarConstantIC.h"

// executioners
#include "Steady.h"
#include "Transient.h"
#include "SolutionTimeAdaptive.h"
#include "DT2Transient.h"
#include "AdaptiveErrorEstimateTransient.h"
#include "TransientAdaptive.h"
#include "CoupledTransientExecutioner.h"

// functions
#include "Composite.h"
#include "MooseParsedFunction.h"
#include "MooseParsedGradFunction.h"
#include "PiecewiseLinear.h"
#include "SolutionFunction.h"
#include "PiecewiseBilinear.h"
#include "PiecewiseLinearFile.h"

// materials
#include "GenericConstantMaterial.h"
#include "GenericFunctionMaterial.h"

// PPS
#include "AverageElementSize.h"
#include "AverageNodalVariableValue.h"
#include "NodalSum.h"
#include "ElementAverageValue.h"
#include "ElementH1Error.h"
#include "ElementH1SemiError.h"
#include "ElementIntegral.h"
#include "ElementIntegralMaterialProperty.h"
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
#include "PrintScalarVariable.h"
#include "Reporter.h"
#include "SideAverageValue.h"
#include "SideFluxIntegral.h"
#include "SideIntegral.h"
#include "NodalMaxValue.h"
#include "NodalProxyMaxValue.h"
#include "PlotFunction.h"
#include "ScalarL2Error.h"
#include "ElementalVariableValue.h"
#include "ElementL2Norm.h"
#include "NodalL2Norm.h"
#include "TotalVariableValue.h"

// user objects
#include "LayeredIntegral.h"
#include "LayeredSideIntegral.h"

// preconditioners
#include "PhysicsBasedPreconditioner.h"
#include "FiniteDifferencePreconditioner.h"
#include "SingleMatrixPreconditioner.h"

// dampers
#include "ConstantDamper.h"
#include "MaxIncrement.h"

// DG
#include "DGDiffusion.h"
#include "DGFunctionDiffusionDirichletBC.h"

// Constraints
#include "TiedValueConstraint.h"
#include "AddBoundsVectorsAction.h"

// ScalarKernels
#include "ODETimeDerivative.h"

// indicators
#include "AnalyticalIndicator.h"
#include "LaplacianJumpIndicator.h"
#include "GradientJumpIndicator.h"
#include "FluxJumpIndicator.h"

// markers
#include "ErrorToleranceMarker.h"
#include "ErrorFractionMarker.h"
#include "UniformMarker.h"
#include "BoxMarker.h"
#include "ComboMarker.h"
#include "ValueThresholdMarker.h"

// Actions
#include "AddMeshModifierAction.h"
#include "AddBCAction.h"
#include "AddDiracKernelAction.h"
#include "AddICAction.h"
#include "AddInitialConditionAction.h"
#include "AddKernelAction.h"
#include "AddScalarKernelAction.h"
#include "AddDGKernelAction.h"
#include "AddPeriodicBCAction.h"
#include "AddVariableAction.h"
#include "AddPostprocessorAction.h"
#include "AddDamperAction.h"
#include "AddFunctionAction.h"
#include "CreateExecutionerAction.h"
#include "SetupTimePeriodsAction.h"
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
#include "SetupDebugAction.h"
#include "SetupResidualDebugAction.h"
#include "InitialRefinementAction.h"
#include "SetupOverSamplingAction.h"
#include "DeprecatedBlockAction.h"
#include "AddConstraintAction.h"
#include "InitDisplacedProblemAction.h"
#include "CreateProblemAction.h"
#include "AddUserObjectAction.h"
#include "AddElementalFieldAction.h"
#include "AddIndicatorAction.h"
#include "AddMarkerAction.h"
#include "SetAdaptivityOptionsAction.h"
#include "AddFEProblemAction.h"
#include "AddCoupledVariableAction.h"

namespace Moose {

static bool registered = false;

void
registerObjects()
{
  if (registered)
    return;

  // mesh
  registerObject(MooseMesh);
  registerObject(GeneratedMesh);
  registerObject(MeshExtruder);

  // problems
  registerProblem(FEProblem);
  registerProblem(OutputProblem);
  registerProblem(CoupledProblem);

  // kernels
  registerKernel(TimeDerivative);
  registerKernel(Diffusion);
  registerKernel(AnisotropicDiffusion);
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

  registerBoundaryCondition(SinDirichletBC);
  registerBoundaryCondition(SinNeumannBC);
  registerBoundaryCondition(VectorNeumannBC);
  registerBoundaryCondition(WeakGradientBC);

  // dirac kernels
  registerDiracKernel(ConstantPointSource);

  // aux kernels
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
  registerAux(BoundsAux);
  registerAux(LayeredIntegralAux);
  registerAux(LayeredSideIntegralAux);

  // Initial Conditions
  registerInitialCondition(ConstantIC);
  registerInitialCondition(BoundingBoxIC);
  registerInitialCondition(FunctionIC);
  registerInitialCondition(RandomIC);
  registerInitialCondition(ScalarConstantIC);

  // executioners
  registerExecutioner(Steady);
  registerExecutioner(Transient);
  registerExecutioner(SolutionTimeAdaptive);
  registerExecutioner(DT2Transient);
  registerExecutioner(AdaptiveErrorEstimateTransient);
  registerExecutioner(CoupledTransientExecutioner);
#if defined(LIBMESH_HAVE_PETSC) && defined(PETSC_VERSION_LE)
#if !PETSC_VERSION_LE(3,3,0)
  registerExecutioner(TransientAdaptive);
#endif
#endif

  // functions
  registerFunction(Composite);
  registerNamedFunction(MooseParsedFunction, "ParsedFunction");
  registerNamedFunction(MooseParsedGradFunction, "ParsedGradFunction");
  registerFunction(PiecewiseLinear);
  registerFunction(SolutionFunction);
  registerFunction(PiecewiseBilinear);
  registerFunction(PiecewiseLinearFile);
  // materials
  registerMaterial(GenericConstantMaterial);
  registerMaterial(GenericFunctionMaterial);

  // PPS
  registerPostprocessor(AverageElementSize);
  registerPostprocessor(AverageNodalVariableValue);
  registerPostprocessor(NodalSum);
  registerPostprocessor(ElementAverageValue);
  registerPostprocessor(ElementH1Error);
  registerPostprocessor(ElementH1SemiError);
  registerPostprocessor(ElementIntegral);
  registerPostprocessor(ElementIntegralMaterialProperty);
  registerPostprocessor(ElementL2Error);
  registerPostprocessor(ScalarL2Error);
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
  registerPostprocessor(PrintScalarVariable);
  registerPostprocessor(PlotFunction);
  registerPostprocessor(Reporter);
  registerPostprocessor(SideAverageValue);
  registerPostprocessor(SideFluxIntegral);
  registerPostprocessor(SideIntegral);
  registerPostprocessor(NodalMaxValue);
  registerPostprocessor(NodalProxyMaxValue);
  registerPostprocessor(ElementalVariableValue);
  registerPostprocessor(ElementL2Norm);
  registerPostprocessor(NodalL2Norm);
  registerPostprocessor(TotalVariableValue);

  // user objects
  registerUserObject(LayeredIntegral);
  registerUserObject(LayeredSideIntegral);

  // preconditioners
  registerNamedPreconditioner(PhysicsBasedPreconditioner, "PBP");
  registerNamedPreconditioner(FiniteDifferencePreconditioner, "FDP");
  registerNamedPreconditioner(SingleMatrixPreconditioner, "SMP");
  // dampers
  registerDamper(ConstantDamper);
  registerDamper(MaxIncrement);
  // DG
  registerDGKernel(DGDiffusion);
  registerBoundaryCondition(DGFunctionDiffusionDirichletBC);

  // Constraints
  registerConstraint(TiedValueConstraint);

  // Scalar kernels
  registerScalarKernel(ODETimeDerivative);

  // indicators
  registerIndicator(AnalyticalIndicator);
  registerIndicator(LaplacianJumpIndicator);
  registerIndicator(GradientJumpIndicator);
  registerIndicator(FluxJumpIndicator);

  // markers
  registerMarker(ErrorToleranceMarker);
  registerMarker(ErrorFractionMarker);
  registerMarker(UniformMarker);
  registerMarker(BoxMarker);
  registerMarker(ComboMarker);
  registerMarker(ValueThresholdMarker);

  registered = true;
}

void
addActionTypes(Syntax & syntax)
{
  /**************************/
  /**** Register Actions ****/
  /**************************/
  /// Minimal Problem
  registerActionName("setup_mesh", false);
  registerActionName("add_variable", false);
  registerActionName("add_kernel", false);
  registerActionName("setup_executioner", true);
  registerActionName("init_displaced_problem", false);
  registerActionName("create_problem", true);
  registerActionName("setup_output", false);
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
  registerActionName("read_mesh", false);
  registerActionName("add_mesh_modifier", false);
  registerActionName("add_extra_nodeset", false);
  registerActionName("setup_time_periods", true);
  registerActionName("add_material", false);
  registerActionName("add_function", false);
  registerActionName("add_aux_variable", false);
  registerActionName("add_aux_kernel", false);
  registerActionName("add_aux_bc", false);
  registerActionName("add_dirac_kernel", false);
  registerActionName("add_scalar_kernel", false);
  registerActionName("add_aux_scalar_kernel", false);
  registerActionName("add_dg_kernel", false);
  registerActionName("add_ic", false);
  registerActionName("add_postprocessor", false);
  registerActionName("add_damper", false);
  registerActionName("add_periodic_bc", false);
  registerActionName("add_preconditioning", false);
  registerActionName("setup_adaptivity", false);
  registerActionName("meta_action", false);
  registerActionName("initial_mesh_refinement", false);
  registerActionName("setup_debug", false);
  registerActionName("setup_residual_debug", false);
  registerActionName("setup_oversampling", false);
  registerActionName("deprecated_block", false);
  registerActionName("add_constraint", false);
  registerActionName("add_user_object", false);
  registerActionName("add_bounds_vectors", false);
  registerActionName("add_elemental_field_variable", false);
  registerActionName("add_indicator", false);
  registerActionName("add_marker", false);
  registerActionName("set_adaptivity_options", false);
  registerActionName("add_feproblem", false);
  registerActionName("add_coupled_variable", false);

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
  syntax.addDependencySets(
"(meta_action)"
"(set_global_params)"
"(read_mesh)"
"(add_extra_nodeset)"
"(setup_mesh)"
"(add_mesh_modifier, setup_mesh_complete)"
"(create_problem)"
"(setup_executioner)"
"(add_feproblem)"
"(setup_time_periods)"
"(init_displaced_problem)"
"(add_aux_variable, add_variable, add_elemental_field_variable)"
"(add_coupled_variable)"
"(setup_variable_complete)"
"(add_user_object)"
"(add_function)"
"(setup_function_complete)"
"(setup_adaptivity)"
"(set_adaptivity_options)"
"(add_ic, add_periodic_bc)"
"(add_preconditioning, add_constraint)"
"(ready_to_init)"
"(setup_quadrature)"
"(setup_dampers)"
"(setup_residual_debug)"
"(add_bounds_vectors)"
"(init_problem)"
"(copy_nodal_vars, copy_nodal_aux_vars)"
"(initial_mesh_refinement)"
"(add_material)"
"(add_postprocessor)"
"(setup_pps_complete)"
"(add_aux_bc, add_aux_kernel, add_bc, add_damper, add_dirac_kernel, add_kernel, add_dg_kernel, add_scalar_kernel, add_aux_scalar_kernel, add_indicator, add_marker, setup_output)"
"(setup_oversampling)"
"(check_integrity)"
"(setup_debug)"
);

}

/**
 * Multiple Action class can be associated with a single input file section, in which case all associated Actions
 * will be created and "acted" on when the associated input file section is seen.
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
registerActions(Syntax & syntax)
{
  registerAction(ReadMeshAction, "read_mesh");
  registerAction(SetupMeshAction, "setup_mesh");
  registerAction(AddExtraNodesetAction, "add_extra_nodeset");

  registerAction(AddFunctionAction, "add_function");
  registerAction(CreateExecutionerAction, "setup_executioner");
  registerAction(SetupTimePeriodsAction, "setup_time_periods");
  registerAction(InitDisplacedProblemAction, "init_displaced_problem");
  registerAction(CreateProblemAction, "create_problem");
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
  registerAction(AddInitialConditionAction, "add_ic");
  registerAction(AddKernelAction, "add_kernel");
  registerAction(AddKernelAction, "add_aux_kernel");
  registerAction(AddScalarKernelAction, "add_scalar_kernel");
  registerAction(AddScalarKernelAction, "add_aux_scalar_kernel");
  registerAction(AddDGKernelAction, "add_dg_kernel");
  registerAction(AddBCAction, "add_bc");
  registerAction(EmptyAction, "no_action");  // placeholder
  registerAction(AddPeriodicBCAction, "add_periodic_bc");
  registerAction(AddBCAction, "add_aux_bc");
  registerAction(AddMaterialAction, "add_material");
  registerAction(AddPostprocessorAction, "add_postprocessor");
  registerAction(AddDamperAction, "add_damper");
  registerAction(SetupPreconditionerAction, "add_preconditioning");
  registerAction(SetupQuadratureAction, "setup_quadrature");
  registerAction(SetupOverSamplingAction, "setup_oversampling");
  registerAction(DeprecatedBlockAction, "deprecated_block");
  registerAction(AddConstraintAction, "add_constraint");
  registerAction(AddUserObjectAction, "add_user_object");
  registerAction(AddElementalFieldAction, "add_elemental_field_variable");
  registerAction(AddIndicatorAction, "add_indicator");
  registerAction(AddMarkerAction, "add_marker");
  registerAction(SetAdaptivityOptionsAction, "set_adaptivity_options");

#ifdef LIBMESH_ENABLE_AMR
  registerAction(AdaptivityAction, "setup_adaptivity");
  registerAction(InitialRefinementAction, "initial_mesh_refinement");
#endif

  registerAction(AddDiracKernelAction, "add_dirac_kernel");
  registerAction(SetupDebugAction, "setup_debug");
  registerAction(SetupResidualDebugAction, "setup_residual_debug");

  registerAction(AddBoundsVectorsAction, "add_bounds_vectors");

  // NonParsedActions
  registerAction(SetupDampersAction, "setup_dampers");
  registerAction(EmptyAction, "ready_to_init");
  registerAction(InitProblemAction, "init_problem");
  registerAction(CheckIntegrityAction, "check_integrity");

  // coupling
  registerAction(AddFEProblemAction, "add_feproblem");
  registerAction(AddCoupledVariableAction, "add_coupled_variable");

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


PerfLog setup_perf_log("Setup");

} // namespace Moose
