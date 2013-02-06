#ifndef SOLIDMECHANICSMODULE_H
#define SOLIDMECHANICSMODULE_H

#include "Factory.h"
#include "Syntax.h"
#include "ActionFactory.h"

namespace Elk
{
  namespace SolidMechanics
  {
    void registerObjects(Factory & factory);
    void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  }
}

#endif //SOLIDMECHANICSMODULE_H
