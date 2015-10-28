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


namespace Moose
{

void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  /**
   * Note: the optional third parameter is used to differentiate which task is
   * satisfied based on the syntax encountered for classes which are registered
   * to satisfy more than one task
   */
  syntax.registerActionSyntax("CopyNodalVarsAction", "Variables/*", "check_copy_nodal_vars");
  syntax.registerActionSyntax("CopyNodalVarsAction", "Variables/*", "copy_nodal_vars");
  syntax.registerActionSyntax("CopyNodalVarsAction", "AuxVariables/*", "check_copy_nodal_vars");
  syntax.registerActionSyntax("CopyNodalVarsAction", "AuxVariables/*", "copy_nodal_aux_vars");

  syntax.registerActionSyntax("AddKernelAction", "Kernels/*", "add_kernel");
  syntax.registerActionSyntax("AddNodalKernelAction", "NodalKernels/*", "add_nodal_kernel");
  syntax.registerActionSyntax("AddKernelAction", "AuxKernels/*", "add_aux_kernel");
  syntax.registerActionSyntax("AddKernelAction", "Bounds/*", "add_aux_kernel");

  syntax.registerActionSyntax("AddScalarKernelAction", "ScalarKernels/*", "add_scalar_kernel");
  syntax.registerActionSyntax("AddScalarKernelAction", "AuxScalarKernels/*", "add_aux_scalar_kernel");

  syntax.registerActionSyntax("AddBCAction", "BCs/*", "add_bc");

  syntax.registerActionSyntax("CreateProblemAction", "Problem");
  syntax.registerActionSyntax("DynamicObjectRegistrationAction", "Problem");
  syntax.registerActionSyntax("SetupMeshAction", "Mesh");
  syntax.registerActionSyntax("SetupMeshCompleteAction", "Mesh");
//  syntax.registerActionSyntax("SetupMeshCompleteAction", "Mesh", "prepare_mesh");
//  syntax.registerActionSyntax("SetupMeshCompleteAction", "Mesh", "setup_mesh_complete");
  syntax.registerActionSyntax("CreateDisplacedProblemAction", "Mesh");
  syntax.registerActionSyntax("AddMeshModifierAction", "MeshModifiers/*");
  syntax.registerActionSyntax("AddMortarInterfaceAction", "Mesh/MortarInterfaces/*");

  syntax.registerActionSyntax("AddFunctionAction", "Functions/*");

  syntax.registerActionSyntax("GlobalParamsAction", "GlobalParams");

  syntax.registerActionSyntax("SetupDebugAction", "Debug");
  syntax.registerActionSyntax("SetupResidualDebugAction", "Debug");

  /// Variable/AuxVariable Actions
  syntax.registerActionSyntax("AddVariableAction", "Variables/*");
//  syntax.registerActionSyntax("AddVariableAction", "Variables/*", "add_variable");
//  syntax.registerActionSyntax("AddVariableAction", "Variables/*", "add_ic");

  syntax.registerActionSyntax("AddICAction", "Variables/*/InitialCondition");

  syntax.registerActionSyntax("AddAuxVariableAction", "AuxVariables/*");
//  syntax.registerActionSyntax("AddAuxVariableAction", "AuxVariables/*", "add_aux_variable");
//  syntax.registerActionSyntax("AddAuxVariableAction", "AuxVariables/*", "add_ic");

  syntax.registerActionSyntax("AddICAction", "AuxVariables/*/InitialCondition");

  syntax.registerActionSyntax("EmptyAction", "BCs/Periodic", "no_action");  // placeholder
  syntax.registerActionSyntax("AddPeriodicBCAction", "BCs/Periodic/*");

  syntax.registerActionSyntax("AddInitialConditionAction", "ICs/*", "add_ic");

  syntax.registerActionSyntax("AddMaterialAction", "Materials/*");

  syntax.registerActionSyntax("SetupPostprocessorDataAction", "Postprocessors/*");
  syntax.registerActionSyntax("AddPostprocessorAction", "Postprocessors/*");

  syntax.registerActionSyntax("AddVectorPostprocessorAction", "VectorPostprocessors/*");

  syntax.registerActionSyntax("AddDamperAction", "Dampers/*");

  syntax.registerActionSyntax("AddOutputAction", "Outputs/*");
  syntax.registerActionSyntax("CommonOutputAction", "Outputs");

  // Note: Preconditioner Actions will be built by this setup action
  syntax.registerActionSyntax("SetupPreconditionerAction", "Preconditioning/*");
  syntax.registerActionSyntax("AddSplitAction","Splits/*");

  syntax.registerActionSyntax("DetermineSystemType", "Executioner");
  syntax.registerActionSyntax("CreateExecutionerAction", "Executioner");
  syntax.registerActionSyntax("SetupTimeStepperAction", "Executioner/TimeStepper");
  syntax.registerActionSyntax("SetupTimeIntegratorAction", "Executioner/TimeIntegrator");

  syntax.registerActionSyntax("SetupTimePeriodsAction", "Executioner/TimePeriods/*");
  syntax.registerActionSyntax("SetupQuadratureAction", "Executioner/Quadrature");
  syntax.registerActionSyntax("SetupPredictorAction", "Executioner/Predictor");
#ifdef LIBMESH_ENABLE_AMR
  syntax.registerActionSyntax("AdaptivityAction", "Executioner/Adaptivity");
#endif

  syntax.registerActionSyntax("PartitionerAction", "Mesh/Partitioner");

  syntax.registerActionSyntax("AddDiracKernelAction", "DiracKernels/*");

  syntax.registerActionSyntax("AddDGKernelAction", "DGKernels/*");

  syntax.registerActionSyntax("AddConstraintAction", "Constraints/*");

  syntax.registerActionSyntax("AddUserObjectAction", "UserObjects/*");
  syntax.registerActionSyntax("AddControlAction", "Controls/*");
  syntax.registerActionSyntax("AddBoundsVectorsAction", "Bounds");

  syntax.registerActionSyntax("AddNodalNormalsAction", "NodalNormals");
//  syntax.registerActionSyntax("AddNodalNormalsAction", "NodalNormals", "add_aux_variable");
//  syntax.registerActionSyntax("AddNodalNormalsAction", "NodalNormals", "add_postprocessor");
//  syntax.registerActionSyntax("AddNodalNormalsAction", "NodalNormals", "add_user_object");

  // Indicator
  syntax.registerActionSyntax("AddElementalFieldAction", "Adaptivity/Indicators/*");
  syntax.registerActionSyntax("AddIndicatorAction", "Adaptivity/Indicators/*");

  // Marker
  syntax.registerActionSyntax("AddElementalFieldAction", "Adaptivity/Markers/*");
  syntax.registerActionSyntax("AddMarkerAction", "Adaptivity/Markers/*");

  // New Adaptivity System
  syntax.registerActionSyntax("SetAdaptivityOptionsAction", "Adaptivity");

  // Deprecated Block
  syntax.registerActionSyntax("DeprecatedBlockAction", "DeprecatedBlock");

  // Multi Apps
  syntax.registerActionSyntax("AddMultiAppAction", "MultiApps/*");

  // Transfers
  syntax.registerActionSyntax("AddTransferAction", "Transfers/*");

  addActionTypes(syntax);
  registerActions(syntax, action_factory);
}


} // namespace
