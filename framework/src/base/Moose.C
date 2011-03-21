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

// Actions
#include "AddAuxBCAction.h"
#include "AddAuxVariableAction.h"
#include "AddBCAction.h"
#include "AddICAction.h"
#include "AddKernelAction.h"
#include "AddPeriodicBCAction.h"
#include "AddVariableAction.h"
#include "CreateExecutionerAction.h"
#include "CreateMeshAction.h"
#include "EmptyAction.h"
#include "InitProblemAction.h"
#include "SetupMeshAction.h"
#include "SetupOutputAction.h"
#include "SetupPeriodicBCAction.h"
#include "AddMaterialAction.h"

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
  registerActionName("add_bc", true);
  registerActionName("add_material", true);
  registerActionName("setup_executioner", true);
  registerActionName("setup_output", true);
  registerActionName("init_problem", true);

  /// Additional Actions
  registerActionName("create_mesh", false);
  registerActionName("modify_mesh", false);
  registerActionName("add_function", false);
  registerActionName("add_aux_variable", false);
  registerActionName("add_aux_kernel", false);
  registerActionName("add_aux_bc", false);
  registerActionName("add_ic", false);
  registerActionName("add_postprocessor", false);
  registerActionName("setup_periodic_bc", false);
  registerActionName("add_periodic_bc", false);

  // Dummy Actions (useful for dependencies)
  registerActionName("setup_mesh_complete", false);
  registerActionName("setup_function_complete", false);
  registerActionName("variable_setup_complete", false);
  registerActionName("setup_pps_complete", false);

  /**************************/
  /****** Dependencies ******/
  /**************************/
  /// Mesh Actions
  action_warehouse.addDependency("setup_mesh", "create_mesh");
  action_warehouse.addDependency("modify_mesh", "setup_mesh");
  action_warehouse.addDependency("setup_mesh_complete", "setup_mesh");
  
  /// Functions
  action_warehouse.addDependency("add_function", "setup_mesh_complete");
  action_warehouse.addDependency("setup_function_complete", "setup_mesh_complete");
  
  /// Variable Actions
  action_warehouse.addDependency("add_variable", "setup_function_complete");
  
  /// AuxVariable Actions
  action_warehouse.addDependency("add_aux_variable", "setup_function_complete");
  action_warehouse.addDependency("variable_setup_complete", "add_variable");

  /// InitProblem
  action_warehouse.addDependency("init_problem", "variable_setup_complete");
  action_warehouse.addDependency("setup_executioner", "init_problem");

  /// Materials
  action_warehouse.addDependency("add_material", "setup_executioner");

  /// Postprocessors
  action_warehouse.addDependency("add_postprocessor", "add_material");
  action_warehouse.addDependency("setup_pps_complete", "add_postprocessor");

  /// ICs
  action_warehouse.addDependency("add_ic", "setup_pps_complete");

  /// Kernels
  action_warehouse.addDependency("add_kernel", "setup_pps_complete");

  /// AuxKernels
  action_warehouse.addDependency("add_aux_kernel", "setup_pps_complete");

  /// BCs
  action_warehouse.addDependency("add_bc", "setup_pps_complete");
  action_warehouse.addDependency("setup_periodic_bc", "setup_pps_complete");
  action_warehouse.addDependency("add_periodic_bc", "setup_periodic_bc");

  /// AuxBCs
  action_warehouse.addDependency("add_aux_bc", "setup_pps_complete");

  

  // Executioner

  action_warehouse.addDependency("setup_output", "setup_pps_complete");
}

void
registerActions()
{
  registerAction(CreateMeshAction, "Mesh/Generation", "create_mesh"); // DONE
  registerAction(SetupMeshAction, "Mesh", "setup_mesh"); // DONE
  registerAction(EmptyAction, "Functions/*", "add_function");  // TODO
  registerAction(AddVariableAction, "Variables/*", "add_variable");   // DONE
  registerAction(AddVariableAction, "AuxVariables/*", "add_aux_variable");  // DONE
  registerAction(AddKernelAction, "Kernels/*", "add_kernel"); // DONE
  registerAction(AddKernelAction, "AuxKernels/*", "add_aux_kernel");  // DONE
  registerAction(AddBCAction, "BCs/*", "add_bc");  // DONE
  registerAction(AddBCAction, "AuxBCs/*", "add_aux_bc");  // DONE
  registerAction(AddMaterialAction, "Materials/*", "add_material");  // DONE
  registerAction(CreateExecutionerAction, "Executioner", "setup_executioner");   //DONE
  registerAction(SetupOutputAction, "Output", "setup_output"); // DONE
  
  
//  registerAction(EmptyAction, "Postprocessors", "no_action");  // REMOVE
  registerAction(EmptyAction, "Postprocessors/*", "no_action");  // TODO
  registerAction(EmptyAction, "Postprocessors/Residual", "no_action");  // TODO
  registerAction(EmptyAction, "Postprocessors/Residual/*", "no_action");  // TODO
  registerAction(EmptyAction, "Postprocessors/Jacobian", "no_action");  // TODO
  registerAction(EmptyAction, "Postprocessors/Jacobian/*", "no_action");  // TODO
  registerAction(EmptyAction, "Postprocessors/NewtonIter", "no_action");  // TODO
  registerAction(EmptyAction, "Postprocessors/NewtonIter/*", "no_action");  // TODO
//  registerAction(EmptyAction, "Dampers", "no_action");  // REMOVE
  registerAction(EmptyAction, "Dampers/*", "no_action");  // TODO
  registerAction(EmptyAction, "GlobalParams", "no_action");  // TODO
//  registerAction(EmptyAction, "DiracKernels", "no_action");  // REMOVE
  registerAction(EmptyAction, "DiracKernels/*", "no_action");  // TODO
  registerAction(EmptyAction, "Preconditioning/PBP", "no_action");  // TODO
  registerAction(EmptyAction, "BCs/Periodic", "no_action");  // TODO
  registerAction(EmptyAction, "BCs/Periodic/*", "no_action");  // TODO
  registerAction(EmptyAction, "Stabilizers/*", "no_action");  // TODO
  registerAction(EmptyAction, "Executioner/Adaptivity", "no_action");   // TODO
  registerAction(EmptyAction, "Variables/*/InitialCondition", "add_ic");   // TODO
  registerAction(EmptyAction, "AuxVariables/*/InitialCondition", "add_ic");  //TODO
  registerAction(EmptyAction, "Mesh/*", "modify_mesh");  // TODO

  registerNonParsedAction(InitProblemAction, "init_problem");
}

void
setSolverDefaults(NonlinearSystem & system)
{
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetDefaults(system);
#endif //LIBMESH_HAVE_PETSC
}

ActionWarehouse action_warehouse;

} // namespace Moose
