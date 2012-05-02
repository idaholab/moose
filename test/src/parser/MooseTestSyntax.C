#include "MooseTestSyntax.h"
#include "Parser.h"
#include "ActionFactory.h"

#include "ConvDiffMetaAction.h"


namespace MooseTest
{

void associateSyntax()
{
  Moose::syntax.registerActionSyntax("ConvDiffMetaAction", "ConvectionDiffusion");

  registerAction(ConvDiffMetaAction, "meta_action");
}

} // namespace
