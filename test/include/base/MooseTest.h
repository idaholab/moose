#ifndef MOOSETEST_H
#define MOOSETEST_H

#include "MooseApp.h"

namespace MooseTest
{
  void registerApps();
  void registerObjects(Factory & factory);
  void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
}

#endif /* MOOSETEST_H */
