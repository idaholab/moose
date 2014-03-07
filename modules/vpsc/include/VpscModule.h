#ifndef VPSCMODULE_H
#define VPSCMODULE_H

#include "Factory.h"
#include "Syntax.h"
#include "ActionFactory.h"

namespace Elk
{
  namespace Vpsc
  {
    void registerObjects(Factory & factory);
    void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  }
}

#endif //VPSCMODULE_H
