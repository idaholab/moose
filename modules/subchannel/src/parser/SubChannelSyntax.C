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
}

}
