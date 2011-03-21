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

  action_warehouse.addAction("no_depends");
  action_warehouse.addAction("no_action");

  /// Mesh Actions
  action_warehouse.addAction("setup_mesh", "no_depends");
  action_warehouse.addAction("create_mesh", "setup_mesh", false);
  action_warehouse.addAction("modify_mesh", "create_mesh", false);

  /// Functions
  action_warehouse.addAction("add_function", "modify_mesh", false);
  
  /// Variable Actions
  action_warehouse.addAction("add_function", "modify_mesh");
  action_warehouse.addAction("add_variable", "variables_block");
  
  /// AuxVariable Actions
  action_warehouse.addAction("aux_variables_block", "add_variable", false);
  action_warehouse.addAction("add_aux_variable", "aux_variables_block", false);

  /// ICs
  action_warehouse.addAction("add_ic", "add_aux_variable", false);

  /// Kernels
  action_warehouse.addAction("kernels_block", "add_ic");
  action_warehouse.addAction("add_kernel", "kernels_block");

  // BCs
  action_warehouse.addAction("bcs_block", "add_kernel");
  action_warehouse.addAction("add_bc", "bcs_block");
  action_warehouse.addAction("setup_periodic_bc", "bcs_block", false);
  action_warehouse.addAction("add_periodic_bc", "setup_periodic_bc", false);

  // AuxBCs
  action_warehouse.addAction("aux_bcs_block", "add_bc", false);
  action_warehouse.addAction("add_aux_bc", "aux_bcs_block", false);

  // Init Problem
  action_warehouse.addAction("init_problem", "setup_mesh", false);
  
  // Executioner
  action_warehouse.addAction("setup_executioner", "init_problem");
  action_warehouse.addAction("setup_output", "setup_executioner");
}

void
registerActions()
{
  registerAction(SetupMeshAction, "Mesh", "setup_mesh");
  registerAction(CreateMeshAction, "Mesh/Generation", "create_mesh");
  registerAction(EmptyAction, "Mesh/*", "no_action");  // TODO

  registerAction(EmptyAction, "Functions", "no_action");
  registerAction(EmptyAction, "Functions/*", "no_action");

  registerAction(EmptyAction, "Variables", "no_action");
  registerAction(AddVariableAction, "Variables/*", "add_variable");

  registerAction(EmptyAction, "Variables/*/InitialCondition", "no_action");
  registerAction(EmptyAction, "AuxVariables", "no_action");
  registerAction(EmptyAction, "AuxVariables/*/InitialCondition", "no_action");
  // Reuse the GenericVariableBlock for AuxVariables/*
  registerAction(EmptyAction, "AuxVariables/*", "no_action");
  registerAction(EmptyAction, "Kernels", "no_action");
  registerAction(EmptyAction, "Kernels/*", "no_action");
//  registerAction(EmptyAction, "DGKernels", "no_action");
//  registerAction(EmptyAction, "DGKernels/*", "no_action");
  registerAction(EmptyAction, "AuxKernels", "no_action");
  registerAction(EmptyAction, "AuxKernels/*", "no_action");
  registerAction(EmptyAction, "BCs", "no_action");
  registerAction(EmptyAction, "BCs/*", "no_action");
  // Reuse the BCsBlock for AuxBCs
  registerAction(EmptyAction, "AuxBCs", "no_action");
  // Reuse the GenericBCBlock for AuxBCs/*
  registerAction(EmptyAction, "AuxBCs/*", "no_action");
  registerAction(EmptyAction, "Stabilizers", "no_action");
  registerAction(EmptyAction, "Stabilizers/*", "no_action");
  registerAction(EmptyAction, "Materials", "no_action");
  registerAction(EmptyAction, "Materials/*", "no_action");
  registerAction(EmptyAction, "Output", "no_action");
  registerAction(EmptyAction, "Preconditioning", "no_action");
  registerAction(EmptyAction, "Preconditioning/PBP", "no_action");
  registerAction(EmptyAction, "BCs/Periodic", "no_action");
  registerAction(EmptyAction, "BCs/Periodic/*", "no_action");
  registerAction(EmptyAction, "Executioner", "no_action");
  registerAction(EmptyAction, "Executioner/Adaptivity", "no_action");
  registerAction(EmptyAction, "Postprocessors", "no_action");
  registerAction(EmptyAction, "Postprocessors/*", "no_action");
  registerAction(EmptyAction, "Postprocessors/Residual", "no_action");
  registerAction(EmptyAction, "Postprocessors/Residual/*", "no_action");
  registerAction(EmptyAction, "Postprocessors/Jacobian", "no_action");
  registerAction(EmptyAction, "Postprocessors/Jacobian/*", "no_action");
  registerAction(EmptyAction, "Postprocessors/NewtonIter", "no_action");
  registerAction(EmptyAction, "Postprocessors/NewtonIter/*", "no_action");
  registerAction(EmptyAction, "Dampers", "no_action");
  registerAction(EmptyAction, "Dampers/*", "no_action");
  registerAction(EmptyAction, "GlobalParams", "no_action");
  registerAction(EmptyAction, "DiracKernels", "no_action");
  registerAction(EmptyAction, "DiracKernels/*", "no_action");
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
