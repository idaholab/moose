#ifndef EXAMPLE_H
#define EXAMPLE_H

namespace Example
{
  /**
   * Registers all Kernels and BCs
   */
  void registerObjects(Factory & factory);
  void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
}

#endif //EXAMPLE_H
