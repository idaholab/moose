#ifndef CHEMICALREACTIONSMODULE_H
#define CHEMICALREACTIONSMODULE_H

#include "Factory.h"
#include "Syntax.h"
#include "ActionFactory.h"

namespace Elk
{
  namespace ChemicalReactions
  {
    void registerObjects(Factory & factory);
    void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  }
}

#endif //CHEMICALREACTIONSMODULE_H
