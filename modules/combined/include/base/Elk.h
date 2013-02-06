#ifndef ELK_H
#define ELK_H

#include "Syntax.h"

namespace Elk
{
  void registerObjects(Factory & factory);
  void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
}

#endif //ELK_H
