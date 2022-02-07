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

  registerSyntax("TriSubChannelBuildMeshAction", "TriSubChannelMesh");
  registerSyntax("AddMeshGeneratorAction", "TriSubChannelMesh/*");
}

}
