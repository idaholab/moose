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
#include "DirichletBC.h"
#include "NeumannBC.h"
#include "FunctionDirichletBC.h"
#include "FunctionNeumannBC.h"
#include "MatchedValueBC.h"
// auxkernels
#include "CoupledAux.h"
#include "ConstantAux.h"
#include "FunctionAux.h"
#include "NearestNodeDistanceAux.h"
#include "NearestNodeValueAux.h"
#include "PenetrationAux.h"
#include "ProcessorIDAux.h"
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
// functions
#include "ParsedFunction.h"
#include "ParsedGradFunction.h"
#include "PiecewiseLinear.h"
#include "SolutionFunction.h"
#include "SphereFunction.h"
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
#include "PrintNumElems.h"
#include "PrintNumNodes.h"
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
#include "AddAuxBCAction.h"
#include "AddAuxVariableAction.h"
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

  registerObject(TimeDerivative);

  // kernels
  registerObject(Diffusion);
  registerObject(CoupledForce);
  registerObject(UserForcingFunction);
  registerObject(BodyForce);
  registerObject(ImplicitEuler);
  registerObject(Reaction);
  registerObject(RealPropertyOutput);
  // bcs
  registerObject(DirichletBC);
  registerObject(NeumannBC);
  registerObject(FunctionDirichletBC);
  registerObject(FunctionNeumannBC);
  registerObject(MatchedValueBC);
  // dirac kernels
  registerObject(ConstantPointSource);
  // aux kernels
  registerObject(CoupledAux);
  registerObject(ConstantAux);
  registerObject(FunctionAux);
  registerObject(NearestNodeDistanceAux);
  registerObject(NearestNodeValueAux);
  registerObject(PenetrationAux);
  registerObject(ProcessorIDAux);
  // Initial Conditions
  registerObject(ConstantIC);
  registerObject(BoundingBoxIC);
  registerObject(FunctionIC);
  registerObject(RandomIC);
  // Mesh Modifiers
  registerObject(ElementDeleter);
  // executioners
  registerObject(Steady);
  registerObject(Transient);
  registerObject(LooseCoupling);
  registerObject(SolutionTimeAdaptive);
  // functions
  registerObject(ParsedFunction);
  registerObject(ParsedGradFunction);
  registerObject(PiecewiseLinear);
  registerObject(SolutionFunction);
  registerObject(SphereFunction);
  // materials
  registerObject(GenericConstantMaterial);
  // PPS
  registerObject(AverageElementSize);
  registerObject(AverageNodalVariableValue);
  registerObject(ElementAverageValue);
  registerObject(ElementH1Error);
  registerObject(ElementH1SemiError);
  registerObject(ElementIntegral);
  registerObject(ElementL2Error);
  registerObject(EmptyPostprocessor);
  registerObject(NodalVariableValue);
  registerObject(PrintDOFs);
  registerObject(PrintDT);
  registerObject(PrintNumElems);
  registerObject(PrintNumNodes);
  registerObject(Reporter);
  registerObject(SideAverageValue);
  registerObject(SideFluxIntegral);
  registerObject(SideIntegral);
  // stabilizers
  registerObject(ConvectionDiffusionSUPG);
  // dampers
  registerObject(ConstantDamper);
  registerObject(MaxIncrement);

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
  registerActionName("add_bc", false);  // Does this need to be true?  Not if you have periodic boundaries...
  registerActionName("setup_dampers", true);
  registerActionName("check_integrity", true);
  
  /// Additional Actions
  registerActionName("no_action", false);  // Used for Empty Action placeholders
  registerActionName("set_global_params", false);
  registerActionName("create_mesh", false);
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

  // Dummy Actions (useful for sync points in the dependencies)
  registerActionName("setup_mesh_complete", false);
  registerActionName("setup_function_complete", false);
  registerActionName("setup_variable_complete", false);
  registerActionName("ready_to_init", false);
  registerActionName("setup_pps_complete", false);

  /**************************/
  /****** Dependencies ******/
  /**************************/
  /// Mesh Actions
  action_warehouse.addDependency("setup_mesh", "create_mesh");
  action_warehouse.addDependency("add_mesh_modifier", "setup_mesh");
  action_warehouse.addDependency("setup_mesh_complete", "setup_mesh");

  /// Executioner
  action_warehouse.addDependency("setup_executioner", "setup_mesh_complete");
  action_warehouse.addDependency("setup_adaptivity", "setup_executioner");
  
  /// Functions
  action_warehouse.addDependency("add_function", "setup_adaptivity");
  action_warehouse.addDependency("setup_function_complete", "add_function");
  
  /// Variable Actions
  action_warehouse.addDependency("add_variable", "setup_function_complete");
  
  /// AuxVariable Actions
  action_warehouse.addDependency("add_aux_variable", "setup_function_complete");
  action_warehouse.addDependency("setup_variable_complete", "add_aux_variable");

  /// ICs
  action_warehouse.addDependency("add_ic", "setup_variable_complete");

  /// PeriodicBCs
  action_warehouse.addDependency("add_periodic_bc", "setup_variable_complete");

  /// Preconditioning
  action_warehouse.addDependency("add_preconditioning", "add_periodic_bc");
  action_warehouse.addDependency("ready_to_init", "add_preconditioning");
  
  /// InitProblem
  action_warehouse.addDependency("setup_dampers", "ready_to_init");
  action_warehouse.addDependency("init_problem", "setup_dampers");
  action_warehouse.addDependency("copy_nodal_vars", "init_problem");

  /// Materials
  action_warehouse.addDependency("add_material", "copy_nodal_vars");

  /// Postprocessors
  action_warehouse.addDependency("add_postprocessor", "add_material");
  action_warehouse.addDependency("setup_pps_complete", "add_postprocessor");

  /// Dampers
  action_warehouse.addDependency("add_damper", "setup_pps_complete");

  /// Stabilizers
  action_warehouse.addDependency("add_stabilizer", "setup_pps_complete");

  /// Kernels
  action_warehouse.addDependency("add_kernel", "setup_pps_complete");

  /// AuxKernels
  action_warehouse.addDependency("add_aux_kernel", "setup_pps_complete");

  /// BCs
  action_warehouse.addDependency("add_bc", "setup_pps_complete");

  /// AuxBCs
  action_warehouse.addDependency("add_aux_bc", "setup_pps_complete");

  /// Dirac Kernels
  action_warehouse.addDependency("add_dirac_kernel", "setup_pps_complete");

  /// Ouput
  action_warehouse.addDependency("setup_output", "setup_pps_complete");

  /// Check Integrity
  action_warehouse.addDependency("check_integrity", "setup_output");
  
}

void
registerActions()
{
  registerAction(CreateMeshAction, "Mesh/Generation", "create_mesh");
  registerAction(SetupMeshAction, "Mesh", "setup_mesh");
  registerAction(AddFunctionAction, "Functions/*", "add_function");
  registerAction(CreateExecutionerAction, "Executioner", "setup_executioner");
  registerAction(SetupOutputAction, "Output", "setup_output");
  registerAction(GlobalParamsAction, "GlobalParams", "set_global_params");
  

  /// MooseObjectActions
  registerAction(AddMeshModifierAction, "Mesh/*", "add_mesh_modifier");
  registerAction(AddVariableAction, "Variables/*", "add_variable");
  registerAction(AddVariableAction, "AuxVariables/*", "add_aux_variable");
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
  registerAction(AdaptivityAction, "Executioner/Adaptivity", "setup_adaptivity");

  registerAction(AddDiracKernelAction, "DiracKernels/*", "add_dirac_kernel");

  // NonParsedActions
  registerNonParsedAction(SetupDampersAction, "setup_dampers");
  registerNonParsedAction(InitProblemAction, "init_problem");
  registerNonParsedAction(CopyNodalVarsAction, "copy_nodal_vars");
  registerNonParsedAction(CheckIntegrityAction, "check_integrity");
}

void
setSolverDefaults(MProblem & problem)
{
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetDefaults(problem);
#endif //LIBMESH_HAVE_PETSC
}

ActionWarehouse action_warehouse;

} // namespace Moose
