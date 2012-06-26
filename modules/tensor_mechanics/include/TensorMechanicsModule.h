#ifndef TENSORMECHANICSMODULE_H
#define TENSORMECHANICSMODULE_H

#include "Syntax.h"

namespace Elk
{
  namespace TensorMechanics
  {
  void registerObjects();

  void associateSyntax(Syntax & syntax);
  }
}

#endif //TENSORMECHANICSMODULE_H
