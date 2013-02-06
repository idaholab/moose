#ifndef HEATCONDUCTIONMODULE_H
#define HEATCONDUCTIONMODULE_H

#include "Syntax.h"

namespace Elk
{
  namespace HeatConduction
  {
    void registerObjects(Factory & factory);
    void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  }
}

#endif //HEATCONDUCTIONMODULE_H
