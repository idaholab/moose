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
#include "Moose.h"


Syntax::Syntax()
{
}

void
Syntax::registerActionSyntax(const std::string & action, const std::string & syntax, const std::string & action_name)
{
  ActionInfo action_info;
  action_info._action = action;
  action_info._action_name = action_name;

  _associated_actions.insert(std::make_pair(syntax, action_info));
}

void
Syntax::replaceActionSyntax(const std::string & action, const std::string & syntax, const std::string & action_name)
{
  _associated_actions.erase(syntax);
  registerActionSyntax(action, syntax, action_name);
}

std::string
Syntax::getSyntaxByAction(const std::string & action, const std::string & action_name)
{
  std::string syntax;
  /**
   * For now we don't have a data structure that maps Actions to Syntax but this routine
   * is only used by the build full tree routine so it doesn't need to be fast.  We
   * will do a linear search for each call to this routine
   */
  for (std::multimap<std::string, ActionInfo>::const_iterator iter = _associated_actions.begin();
       iter != _associated_actions.end(); ++iter)
  {
    if (iter->second._action == action &&
        (iter->second._action_name == action_name || iter->second._action_name == ""))
      syntax = iter->first;
  }

  return syntax;
}

std::string
Syntax::isAssociated(const std::string & real_id, bool * is_parent)
{
  /**
   * This implementation assumes that wildcards can occur in the place of an entire token but not as part
   * of a token (i.e.  'Variables/ * /InitialConditions' is valid but not 'Variables/Partial* /InitialConditions'.
   * Since maps are ordered, a reverse traversal through the registered list will always select a more
   * specific match before a wildcard match ('*' == char(42))
   */
  bool local_is_parent;
  if (is_parent == NULL)
   is_parent = &local_is_parent;  // Just so we don't have to keep checking below when we want to set the value
  std::multimap<std::string, ActionInfo>::reverse_iterator it;
  std::vector<std::string> real_elements, reg_elements;
  std::string return_value;

  Parser::tokenize(real_id, real_elements);

  *is_parent = false;
  for (it=_associated_actions.rbegin(); it != _associated_actions.rend(); ++it)
  {
    std::string reg_id = it->first;
    if (reg_id == real_id)
    {
      *is_parent = false;
      return reg_id;
    }
    reg_elements.clear();
    Parser::tokenize(reg_id, reg_elements);
    if (real_elements.size() <= reg_elements.size())
    {
      bool keep_going = true;
      for (unsigned int j=0; keep_going && j<real_elements.size(); ++j)
      {
        if (real_elements[j] != reg_elements[j] && reg_elements[j] != std::string("*"))
          keep_going = false;
      }
      if (keep_going)
      {
        if (real_elements.size() < reg_elements.size() && !*is_parent)
        {
          // We found a parent, the longest parent in fact but we need to keep
          // looking to make sure that the real thing isn't registered
          *is_parent = true;
          return_value = reg_id;
        }
        else if (real_elements.size() == reg_elements.size())
        {
          *is_parent = false;
          return reg_id;
        }
      }
    }
  }

  if (*is_parent)
    return return_value;
  else
    return std::string("");
}

std::pair<std::multimap<std::string, Syntax::ActionInfo>::iterator, std::multimap<std::string, Syntax::ActionInfo>::iterator>
Syntax::getActions(const std::string & name)
{
  return _associated_actions.equal_range(name);
}

/////////////////////////////////////////////////////////////////////////////////////////

namespace Moose
{

Syntax syntax;

void associateSyntax()
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

  addActionTypes();
  registerActions();
}


} // namespace
