#ifndef MISCMODULE_H
#define MISCMODULE_H

#include "Syntax.h"

namespace Elk
{
  namespace Misc
  {
    void registerObjects(Factory & factory);
    void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  }
}

#endif //MISCMODULE_H
