#ifndef TENSORMECHANICSMODULE_H
#define TENSORMECHANICSMODULE_H

#include "Syntax.h"

namespace Elk
{
  namespace TensorMechanics
  {
    void registerObjects(Factory & factory);
    void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  }
}

#endif //TENSORMECHANICSMODULE_H
