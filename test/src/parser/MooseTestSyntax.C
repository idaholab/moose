#include "MooseTestSyntax.h"
#include "Parser.h"

namespace MooseTest
{
void associateSyntax(Parser & p)
{
  p.registerActionSyntax("ConvDiffMetaAction", "ConvectionDiffusion");
}

} // namespace
