//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
