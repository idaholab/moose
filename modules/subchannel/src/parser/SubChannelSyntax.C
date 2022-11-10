/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "SubChannelSyntax.h"
#include "ActionFactory.h"
#include "Syntax.h"

namespace SubChannel
{

void
associateSyntax(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerSyntax("SubChannelAddVariablesAction", "SubChannel");
  registerSyntax("SubChannelCreateProblemAction", "SubChannel");

  registerTask("sch:build_subchannel_mesh", false);

  try
  {
    syntax.addDependency("sch:build_subchannel_mesh", "check_copy_nodal_vars");
  }
  catch (CyclicDependencyException<std::string> & e)
  {
    mooseError("Cyclic Dependency Detected during addDependency() calls");
  }

  registerSyntax("QuadSubChannelBuildMeshAction", "QuadSubChannelMesh");
  registerSyntax("AddMeshGeneratorAction", "QuadSubChannelMesh/*");

  registerSyntax("QuadInterWrapperBuildMeshAction", "QuadInterWrapperMesh");
  registerSyntax("AddMeshGeneratorAction", "QuadInterWrapperMesh/*");

  registerSyntax("TriSubChannelBuildMeshAction", "TriSubChannelMesh");
  registerSyntax("AddMeshGeneratorAction", "TriSubChannelMesh/*");

  registerSyntax("TriInterWrapperBuildMeshAction", "TriInterWrapperMesh");
  registerSyntax("AddMeshGeneratorAction", "TriInterWrapperMesh/*");
}

}
