#ifndef COMBINEDTESTAPP_H
#define COMBINEDTESTAPP_H

#include "CombinedApp.h"

class CombinedTestApp;

template <>
InputParameters validParams<CombinedTestApp>();

class CombinedTestApp : public CombinedApp
{
public:
  CombinedTestApp(const InputParameters & parameters);
  virtual ~CombinedTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* COMBINEDTESTAPP_H */
