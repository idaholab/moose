#ifndef EX15APP_H
#define EX15APP_H

#include "MooseApp.h"

/**
 * Ex15 application object to demonstrate how to inherit from MooseApp class
 * and design the user application
 */
class Ex15App : public MooseApp
{
public:
  Ex15App(int argc, char * argv[]);

  /// Register objects used by this application (kernels, bcs, actions, ...)
  static void registerObjects();
  /// Associate new syntax (this is static since all applications can be compiled
  /// as libraries and linked to other applications which will create their own
  /// MooseApp derived object)
  static void associateSyntax(Syntax & syntax);
};

#endif /* EX15APP_H */
