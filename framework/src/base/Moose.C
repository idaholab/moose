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

// objects that can be created by MOOSE
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
#include "GapValueAux.h"

// dirac kernels
#include "ConstantPointSource.h"
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
#include "PrintResidual.h"
#include "Reporter.h"
#include "SideAverageValue.h"
#include "SideFluxIntegral.h"
#include "SideIntegral.h"
// stabilizers
#include "ConvectionDiffusionSUPG.h"
// dampers
#include "ConstantDamper.h"
#include "MaxIncrement.h"

// Actions
#include "AddMeshModifierAction.h"
#include "AddBCAction.h"
#include "AddDiracKernelAction.h"
#include "AddICAction.h"
#include "AddKernelAction.h"
#include "AddPeriodicBCAction.h"
#include "AddVariableAction.h"
#include "AddPostprocessorAction.h"
#include "AddDamperAction.h"
#include "AddStabilizerAction.h"
#include "AddFunctionAction.h"
#include "CreateExecutionerAction.h"
#include "CreateMeshAction.h"
#include "ReadMeshAction.h"
#include "EmptyAction.h"
#include "InitProblemAction.h"
#include "CopyNodalVarsAction.h"
#include "SetupMeshAction.h"
#include "SetupOutputAction.h"
#include "AddMaterialAction.h"
#include "GlobalParamsAction.h"
#include "SetupPBPAction.h"
#include "AdaptivityAction.h"
#include "SetupDampersAction.h"
#include "CheckIntegrityAction.h"

namespace Moose {

static bool registered = false;

void
registerObjects()
{
  if (registered)
    return;

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
  registerAux(GapValueAux);

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
  registerPostprocessor(PrintResidual);
  registerPostprocessor(Reporter);
  registerPostprocessor(SideAverageValue);
  registerPostprocessor(SideFluxIntegral);
  registerPostprocessor(SideIntegral);
  // stabilizers
  registerStabilizer(ConvectionDiffusionSUPG);
  // dampers
  registerDamper(ConstantDamper);
  registerDamper(MaxIncrement);

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
  registerActionName("setup_output", true);
  registerActionName("init_problem", true);
  registerActionName("copy_nodal_vars", true);
  registerActionName("copy_nodal_aux_vars", true);
  registerActionName("add_bc", false);  // Does this need to be true?  Not if you have periodic boundaries...
  registerActionName("setup_dampers", true);
  registerActionName("check_integrity", true);

  /// Additional Actions
  registerActionName("no_action", false);  // Used for Empty Action placeholders
  registerActionName("set_global_params", false);
  registerActionName("create_mesh", false);
  registerActionName("read_mesh", false);
  registerActionName("add_mesh_modifier", false);
  registerActionName("add_material", false);
  registerActionName("add_function", false);
  registerActionName("add_aux_variable", false);
  registerActionName("add_aux_kernel", false);
  registerActionName("add_aux_bc", false);
  registerActionName("add_dirac_kernel", false);
  registerActionName("add_ic", false);
  registerActionName("add_postprocessor", false);
  registerActionName("add_damper", false);
  registerActionName("add_stabilizer", false);
  registerActionName("add_periodic_bc", false);
  registerActionName("add_preconditioning", false);
  registerActionName("setup_adaptivity", false);
  registerActionName("meta_action", false);

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
"(create_mesh, read_mesh, set_global_params)"
"(setup_mesh)"
"(add_mesh_modifier, setup_mesh_complete)"
"(setup_executioner)"
"(add_function)"
"(setup_function_complete)"
"(add_aux_variable, add_variable)"
"(setup_variable_complete)"
"(setup_adaptivity)"
"(add_ic, add_periodic_bc)"
"(add_preconditioning)"
"(ready_to_init)"
"(setup_dampers)"
"(init_problem)"
"(copy_nodal_vars, copy_nodal_aux_vars)"
"(add_material)"
"(add_postprocessor)"
"(setup_pps_complete)"
"(add_aux_bc, add_aux_kernel, add_bc, add_damper, add_dirac_kernel, add_kernel, add_stabilizer, setup_output)"
"(check_integrity)"
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
 */
void
registerActions()
{
  registerAction(CreateMeshAction, "Mesh/Generation", "create_mesh");
  registerAction(ReadMeshAction, "Mesh", "read_mesh");  
  registerAction(SetupMeshAction, "Mesh", "setup_mesh");
  registerAction(AddFunctionAction, "Functions/*", "add_function");
  registerAction(CreateExecutionerAction, "Executioner", "setup_executioner");
  registerAction(SetupOutputAction, "Output", "setup_output");
  registerAction(GlobalParamsAction, "GlobalParams", "set_global_params");

  /// MooseObjectActions
  registerAction(AddMeshModifierAction, "Mesh/*", "add_mesh_modifier");

  /// Variable/AuxVariable Actions
  registerAction(AddVariableAction, "Variables/*", "add_variable");
  registerAction(CopyNodalVarsAction, "Variables/*", "copy_nodal_vars");
  registerAction(AddVariableAction, "AuxVariables/*", "add_aux_variable");
  registerAction(CopyNodalVarsAction, "AuxVariables/*", "copy_nodal_aux_vars");

  registerAction(AddICAction, "Variables/*/InitialCondition", "add_ic");
  registerAction(AddICAction, "AuxVariables/*/InitialCondition", "add_ic");
  registerAction(AddKernelAction, "Kernels/*", "add_kernel");
  registerAction(AddKernelAction, "AuxKernels/*", "add_aux_kernel");
  registerAction(AddBCAction, "BCs/*", "add_bc");
  registerAction(EmptyAction, "BCs/Periodic", "no_action");  // placeholder
  registerAction(AddPeriodicBCAction, "BCs/Periodic/*", "add_periodic_bc");
  registerAction(AddBCAction, "AuxBCs/*", "add_aux_bc");
  registerAction(AddMaterialAction, "Materials/*", "add_material");
  registerAction(AddPostprocessorAction, "Postprocessors/*", "add_postprocessor");
  registerAction(EmptyAction, "Postprocessors/Residual", "no_action");   // placeholder
  registerAction(EmptyAction, "Postprocessors/Jacobian", "no_action");   // placeholder
  registerAction(EmptyAction, "Postprocessors/NewtonIter", "no_action"); // placeholder
  registerAction(AddPostprocessorAction, "Postprocessors/Residual/*", "add_postprocessor");
  registerAction(AddPostprocessorAction, "Postprocessors/Jacobian/*", "add_postprocessor");
  registerAction(AddPostprocessorAction, "Postprocessors/NewtonIter/*", "add_postprocessor");
  registerAction(AddDamperAction, "Dampers/*", "add_damper");
  registerAction(AddStabilizerAction, "Stabilizers/*", "add_stabilizer");
  registerAction(SetupPBPAction, "Preconditioning/PBP", "add_preconditioning");

#ifdef LIBMESH_ENABLE_AMR
  registerAction(AdaptivityAction, "Executioner/Adaptivity", "setup_adaptivity");
#endif

  registerAction(AddDiracKernelAction, "DiracKernels/*", "add_dirac_kernel");

  // NonParsedActions
  registerNonParsedAction(SetupDampersAction, "setup_dampers");
  registerNonParsedAction(EmptyAction, "ready_to_init");
  registerNonParsedAction(InitProblemAction, "init_problem");
  registerNonParsedAction(CheckIntegrityAction, "check_integrity");

  registerActionName("finish_input_file_output", false);
  registerNonParsedAction(EmptyAction, "finish_input_file_output");
}

void
setSolverDefaults(MProblem & problem)
{
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetDefaults(problem);
#endif //LIBMESH_HAVE_PETSC
}

ActionWarehouse action_warehouse;

PerfLog setup_perf_log("Setup");

} // namespace Moose
