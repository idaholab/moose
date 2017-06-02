#ifndef CONTROL_TESTAPP_H
#define CONTROL_TESTAPP_H

#include "MooseApp.h"

class PythonControlApp;

template <>
InputParameters validParams<PythonControlApp>();

/**
 * A class to demostrate and test using PythonControl
 */
class PythonControlApp : public MooseApp
{
public:
  PythonControlApp(InputParameters parameters);

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* CONTROL_TESTAPP_H */
