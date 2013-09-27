#ifndef PHASEFIELDMODULE_H
#define PHASEFIELDMODULE_H

#include "Factory.h"
#include "Syntax.h"
#include "ActionFactory.h"

namespace Elk
{
  namespace PhaseField
  {
    void registerObjects(Factory & factory);
    void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  }
}

#endif //PHASEFIELDMODULE_H
