#ifndef SOLIDMECHANICSMODULE_H
#define SOLIDMECHANICSMODULE_H

#include "Syntax.h"

namespace Elk
{
  namespace SolidMechanics
  {
    void registerObjects(Factory & factory);
    void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  }
}

#endif //SOLIDMECHANICSMODULE_H
