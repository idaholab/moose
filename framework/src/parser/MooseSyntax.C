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
   * Note: the optional third parameter is used to differentiate which action_name is
   * satisfied based on the syntax encountered for classes which are registered
   * to satisfy more than one action_name
   */
  syntax.registerActionSyntax("CreateProblemAction", "Problem");
  syntax.registerActionSyntax("ReadMeshAction", "Mesh");
  syntax.registerActionSyntax("SetupMeshAction", "Mesh");
  syntax.registerActionSyntax("InitialRefinementAction", "Mesh");
  syntax.registerActionSyntax("InitDisplacedProblemAction", "Mesh");
  syntax.registerActionSyntax("AddExtraNodesetAction", "Mesh/ExtraNodesets/*");

  syntax.registerActionSyntax("AddFunctionAction", "Functions/*");

  syntax.registerActionSyntax("GlobalParamsAction", "GlobalParams");

  syntax.registerActionSyntax("SetupDebugAction", "Debug");
  syntax.registerActionSyntax("SetupResidualDebugAction", "Debug");

  /// Variable/AuxVariable Actions
  syntax.registerActionSyntax("AddVariableAction", "Variables/*", "add_variable");
  syntax.registerActionSyntax("CopyNodalVarsAction", "Variables/*", "copy_nodal_vars");
  syntax.registerActionSyntax("AddICAction", "Variables/*/InitialCondition");

  syntax.registerActionSyntax("AddVariableAction", "AuxVariables/*", "add_aux_variable");
  syntax.registerActionSyntax("CopyNodalVarsAction", "AuxVariables/*", "copy_nodal_aux_vars");
  syntax.registerActionSyntax("AddICAction", "AuxVariables/*/InitialCondition");

  syntax.registerActionSyntax("AddKernelAction", "Kernels/*", "add_kernel");

  syntax.registerActionSyntax("AddKernelAction", "AuxKernels/*", "add_aux_kernel");

  syntax.registerActionSyntax("AddScalarKernelAction", "ScalarKernels/*", "add_scalar_kernel");

  syntax.registerActionSyntax("AddScalarKernelAction", "AuxScalarKernels/*", "add_aux_scalar_kernel");

  syntax.registerActionSyntax("AddBCAction", "BCs/*", "add_bc");
  syntax.registerActionSyntax("EmptyAction", "BCs/Periodic");  // placeholder
  syntax.registerActionSyntax("AddPeriodicBCAction", "BCs/Periodic/*");

  syntax.registerActionSyntax("AddBCAction", "AuxBCs/*", "add_aux_bc");

  syntax.registerActionSyntax("AddInitialConditionAction", "ICs/*", "add_ic");

  syntax.registerActionSyntax("AddMaterialAction", "Materials/*");

  syntax.registerActionSyntax("AddPostprocessorAction", "Postprocessors/*");

  syntax.registerActionSyntax("AddDamperAction", "Dampers/*");

  syntax.registerActionSyntax("SetupOutputAction", "Output");
  syntax.registerActionSyntax("SetupOverSamplingAction", "Output/OverSampling");

  // Note: Preconditioner Actions will be built by this setup action
  syntax.registerActionSyntax("SetupPreconditionerAction", "Preconditioning/*");

  syntax.registerActionSyntax("CreateExecutionerAction", "Executioner");
  syntax.registerActionSyntax("SetupTimePeriodsAction", "Executioner/TimePeriods/*");
  syntax.registerActionSyntax("SetupQuadratureAction", "Executioner/Quadrature");
#ifdef LIBMESH_ENABLE_AMR
  syntax.registerActionSyntax("AdaptivityAction", "Executioner/Adaptivity");
#endif

  syntax.registerActionSyntax("AddDiracKernelAction", "DiracKernels/*");

  syntax.registerActionSyntax("AddDGKernelAction", "DGKernels/*");

  syntax.registerActionSyntax("AddConstraintAction", "Constraints/*");

  syntax.registerActionSyntax("AddUserObjectAction", "UserObjects/*", "add_user_object");

  syntax.registerActionSyntax("AddBoundsVectorsAction", "Bounds", "add_bounds_vectors");

  // This works because the AddKernelAction will build AuxKernels if the path doesn't contain Kernels!
  syntax.registerActionSyntax("AddKernelAction", "Bounds/*", "add_aux_kernel");

  // Coupling
  syntax.registerActionSyntax("AddFEProblemAction", "CoupledProblems/*");
  syntax.registerActionSyntax("AddCoupledVariableAction", "CoupledProblems/*/*");

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
  syntax.registerActionSyntax("AddMultiAppAction", "MultiApps/*", "add_multi_app");

  // Transfers
  syntax.registerActionSyntax("AddTransferAction", "Transfers/*", "add_transfer");

  addActionTypes(syntax);
  registerActions(syntax, action_factory);
}


} // namespace
