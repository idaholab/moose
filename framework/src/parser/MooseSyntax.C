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
#include "Parser.h"

namespace Moose
{

void associateSyntax(Parser & p)
{
  /**
   * Note: the optional third parameter is used to differentiate which action_name is
   * satisified based on the syntax encountered for classes which are registered
   * to satisfy more than one action_name
   */
  p.registerActionSyntax("CreateMeshAction", "Mesh/Generation");
  p.registerActionSyntax("ReadMeshAction", "Mesh");
  p.registerActionSyntax("SetupMeshAction", "Mesh");
  p.registerActionSyntax("AddExtraNodesetAction", "Mesh/ExtraNodesets/*");
  
  p.registerActionSyntax("AddFunctionAction", "Functions/*");
  p.registerActionSyntax("CreateExecutionerAction", "Executioner");
  p.registerActionSyntax("SetupOutputAction", "Output");
  p.registerActionSyntax("GlobalParamsAction", "GlobalParams");
  p.registerActionSyntax("SetupDebugAction", "Debug");

  /// MooseObjectActions
  p.registerActionSyntax("AddMeshModifierAction", "Mesh/Modifier/*");

  /// Variable/AuxVariable Actions
  p.registerActionSyntax("AddVariableAction", "Variables/*", "add_variable");
  p.registerActionSyntax("CopyNodalVarsAction", "Variables/*", "copy_nodal_vars");
  p.registerActionSyntax("AddVariableAction", "AuxVariables/*", "add_aux_variable");
  p.registerActionSyntax("CopyNodalVarsAction", "AuxVariables/*", "copy_nodal_aux_vars");
  
  p.registerActionSyntax("AddICAction", "Variables/*/InitialCondition");
  p.registerActionSyntax("AddICAction", "AuxVariables/*/InitialCondition");
  
  p.registerActionSyntax("AddKernelAction", "Kernels/*", "add_kernel");
  p.registerActionSyntax("AddKernelAction", "AuxKernels/*", "add_aux_kernel");
  
  p.registerActionSyntax("AddBCAction", "BCs/*", "add_bc");
  p.registerActionSyntax("AddBCAction", "AuxBCs/*", "add_aux_bc");
  
  p.registerActionSyntax("EmptyAction", "BCs/Periodic");  // placeholder
  p.registerActionSyntax("AddPeriodicBCAction", "BCs/Periodic/*");
  p.registerActionSyntax("AddMaterialAction", "Materials/*");
  p.registerActionSyntax("AddPostprocessorAction", "Postprocessors/*");
  p.registerActionSyntax("EmptyAction", "Postprocessors/Residual");   // placeholder
  p.registerActionSyntax("EmptyAction", "Postprocessors/Jacobian");   // placeholder
  p.registerActionSyntax("EmptyAction", "Postprocessors/NewtonIter"); // placeholder
  p.registerActionSyntax("AddPostprocessorAction", "Postprocessors/Residual/*");
  p.registerActionSyntax("AddPostprocessorAction", "Postprocessors/Jacobian/*");
  p.registerActionSyntax("AddPostprocessorAction", "Postprocessors/NewtonIter/*");
  p.registerActionSyntax("AddDamperAction", "Dampers/*");

  // Note: Preconditioner Actions will be built by this setup action
  p.registerActionSyntax("SetupPreconditionerAction", "Preconditioning/*");
  p.registerActionSyntax("SetupQuadratureAction", "Executioner/Quadrature");

#ifdef LIBMESH_ENABLE_AMR
  p.registerActionSyntax("AdaptivityAction", "Executioner/Adaptivity");
#endif
  
  p.registerActionSyntax("AddDiracKernelAction", "DiracKernels/*");
}

  
} // namespace
