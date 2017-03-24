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

#include "MooseSyntax.h"
#include "Syntax.h"
#include "Moose.h"
#include "ActionFactory.h"

namespace Moose
{

void
associateSyntax(Syntax & syntax, ActionFactory & action_factory)
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

  // Note: Preconditioner Actions will be built by this setup action
  registerSyntax("SetupPreconditionerAction", "Preconditioning/*");
  registerSyntax("AddFieldSplitAction", "Preconditioning/*/*");

  registerSyntax("DetermineSystemType", "Executioner");
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

  // Marker
  registerSyntax("AddElementalFieldAction", "Adaptivity/Markers/*");
  registerSyntax("AddMarkerAction", "Adaptivity/Markers/*");

  // New Adaptivity System
  registerSyntax("SetAdaptivityOptionsAction", "Adaptivity");

  // Deprecated Block
  registerSyntax("DeprecatedBlockAction", "DeprecatedBlock");

  // Multi Apps
  registerSyntax("AddMultiAppAction", "MultiApps/*");

  // Transfers
  registerSyntax("AddTransferAction", "Transfers/*");

  addActionTypes(syntax);
  registerActions(syntax, action_factory);
}

} // namespace
