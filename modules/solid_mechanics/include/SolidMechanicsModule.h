#ifndef SOLIDMECHANICSMODULE_H
#define SOLIDMECHANICSMODULE_H

#include "Syntax.h"

namespace Elk
{
  namespace SolidMechanics
  {
  void registerObjects();

  void associateSyntax(Syntax & syntax);
  }
}

#endif //SOLIDMECHANICSMODULE_H
