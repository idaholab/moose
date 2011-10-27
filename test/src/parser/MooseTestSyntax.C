#include "MooseTestSyntax.h"
#include "Parser.h"

namespace MooseTest
{

void associateSyntax()
{
  Moose::syntax.registerActionSyntax("ConvDiffMetaAction", "ConvectionDiffusion");
}

} // namespace
