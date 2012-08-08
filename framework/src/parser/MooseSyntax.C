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
#include "Moose.h"


namespace Moose
{

void associateSyntax(Syntax & syntax)
{
  /**
   * Note: the optional third parameter is used to differentiate which action_name is
   * satisfied based on the syntax encountered for classes which are registered
   * to satisfy more than one action_name
   */
  syntax.registerActionSyntax("SetupSubProblemAction", "Problem");
  syntax.registerActionSyntax("ReadMeshAction", "Mesh");
  syntax.registerActionSyntax("SetupMeshAction", "Mesh");
  syntax.registerActionSyntax("InitialRefinementAction", "Mesh");
  syntax.registerActionSyntax("InitDisplacedProblemAction", "Mesh");
//  syntax.registerActionSyntax("DeprecatedBlockAction", "Mesh/Generation");
//  syntax.registerActionSyntax("EmptyAction", "Mesh/ExtraNodesets");
  syntax.registerActionSyntax("AddExtraNodesetAction", "Mesh/ExtraNodesets/*");
  syntax.registerActionSyntax("AddMeshModifierAction", "Mesh/Modifier/*");

//  syntax.registerActionSyntax("EmptyAction", "Functions");
  syntax.registerActionSyntax("AddFunctionAction", "Functions/*");

  syntax.registerActionSyntax("GlobalParamsAction", "GlobalParams");

  syntax.registerActionSyntax("SetupDebugAction", "Debug");
  syntax.registerActionSyntax("SetupResidualDebugAction", "Debug");

  /// Variable/AuxVariable Actions
//  syntax.registerActionSyntax("EmptyAction", "Variables");
  syntax.registerActionSyntax("AddVariableAction", "Variables/*", "add_variable");
  syntax.registerActionSyntax("CopyNodalVarsAction", "Variables/*", "copy_nodal_vars");
  syntax.registerActionSyntax("AddICAction", "Variables/*/InitialCondition");

//  syntax.registerActionSyntax("EmptyAction", "AuxVariables");
  syntax.registerActionSyntax("AddVariableAction", "AuxVariables/*", "add_aux_variable");
  syntax.registerActionSyntax("CopyNodalVarsAction", "AuxVariables/*", "copy_nodal_aux_vars");
  syntax.registerActionSyntax("AddICAction", "AuxVariables/*/InitialCondition");

//  syntax.registerActionSyntax("EmptyAction", "Kernels");
  syntax.registerActionSyntax("AddKernelAction", "Kernels/*", "add_kernel");

//  syntax.registerActionSyntax("EmptyAction", "AuxKernels");
  syntax.registerActionSyntax("AddKernelAction", "AuxKernels/*", "add_aux_kernel");

//  syntax.registerActionSyntax("EmptyAction", "ScalarKernels");
  syntax.registerActionSyntax("AddScalarKernelAction", "ScalarKernels/*", "add_scalar_kernel");

//  syntax.registerActionSyntax("EmptyAction", "AuxScalarKernels");
  syntax.registerActionSyntax("AddScalarKernelAction", "AuxScalarKernels/*", "add_aux_scalar_kernel");

//  syntax.registerActionSyntax("EmptyAction", "BCs");
  syntax.registerActionSyntax("AddBCAction", "BCs/*", "add_bc");
  syntax.registerActionSyntax("EmptyAction", "BCs/Periodic");  // placeholder
  syntax.registerActionSyntax("AddPeriodicBCAction", "BCs/Periodic/*");

//  syntax.registerActionSyntax("EmptyAction", "AuxBCs");
  syntax.registerActionSyntax("AddBCAction", "AuxBCs/*", "add_aux_bc");

//  syntax.registerActionSyntax("EmptyAction", "ICs");
  syntax.registerActionSyntax("AddInitialConditionAction", "ICs/*", "add_ic");

//  syntax.registerActionSyntax("EmptyAction", "Materials");
  syntax.registerActionSyntax("AddMaterialAction", "Materials/*");

//  syntax.registerActionSyntax("EmptyAction", "Postprocessors");
  syntax.registerActionSyntax("AddPostprocessorAction", "Postprocessors/*");

//  syntax.registerActionSyntax("DeprecatedBlockAction", "Postprocessors/Residual");
//  syntax.registerActionSyntax("DeprecatedBlockAction", "Postprocessors/Jacobian");
//  syntax.registerActionSyntax("DeprecatedBlockAction", "Postprocessors/NewtonIter");
//  syntax.registerActionSyntax("DeprecatedBlockAction", "Postprocessors/Residual/*");
//  syntax.registerActionSyntax("DeprecatedBlockAction", "Postprocessors/Jacobian/*");
//  syntax.registerActionSyntax("DeprecatedBlockAction", "Postprocessors/NewtonIter/*");

//  syntax.registerActionSyntax("EmptyAction", "Dampers");
  syntax.registerActionSyntax("AddDamperAction", "Dampers/*");

  syntax.registerActionSyntax("SetupOutputAction", "Output");
  syntax.registerActionSyntax("SetupOverSamplingAction", "Output/OverSampling");

  // Note: Preconditioner Actions will be built by this setup action
//  syntax.registerActionSyntax("EmptyAction", "Preconditioning");
  syntax.registerActionSyntax("SetupPreconditionerAction", "Preconditioning/*");

  syntax.registerActionSyntax("CreateExecutionerAction", "Executioner");
  syntax.registerActionSyntax("SetupTimePeriodsAction", "Executioner/TimePeriods/*");
  syntax.registerActionSyntax("SetupQuadratureAction", "Executioner/Quadrature");
#ifdef LIBMESH_ENABLE_AMR
  syntax.registerActionSyntax("AdaptivityAction", "Executioner/Adaptivity");
#endif

//  syntax.registerActionSyntax("EmptyAction", "DiracKernels");
  syntax.registerActionSyntax("AddDiracKernelAction", "DiracKernels/*");

//  syntax.registerActionSyntax("EmptyAction", "DGKernels");
  syntax.registerActionSyntax("AddDGKernelAction", "DGKernels/*");

//  syntax.registerActionSyntax("EmptyAction", "Constraints");
  syntax.registerActionSyntax("AddConstraintAction", "Constraints/*");

//  syntax.registerActionSyntax("EmptyAction", "UserObjects");
  syntax.registerActionSyntax("AddUserObjectAction", "UserObjects/*", "add_user_object");

  syntax.registerActionSyntax("AddBoundsVectorsAction", "Bounds", "add_bounds_vectors");

  // This works because the AddKernelAction will build AuxKernels if the path doesn't contain Kernels!
  syntax.registerActionSyntax("AddKernelAction", "Bounds/*", "add_aux_kernel");

  // Loose Coupling
  syntax.registerActionSyntax("EmptyAction", "SubProblems");

  addActionTypes(syntax);
  registerActions(syntax);
}


} // namespace
