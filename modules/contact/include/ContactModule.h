#ifndef CONTACTMODULE_H
#define CONTACTMODULE_H

#include "Syntax.h"

namespace Elk
{
  namespace Contact
  {
    void registerObjects(Factory & factory);
    void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  }
}

#endif //CONTACTMODULE_H
