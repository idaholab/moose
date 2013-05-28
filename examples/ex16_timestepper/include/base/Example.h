#ifndef EXAMPLE_H
#define EXAMPLE_H

class Factory;
class ActionFactory;
class Syntax;

namespace Example
{
  /**
   * Register this application and any it depends on.
   */
  void registerApps();
  /**
   * Registers all Kernels and BCs
   */
  void registerObjects(Factory & factory);
  void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
}

#endif //EXAMPLE_H
