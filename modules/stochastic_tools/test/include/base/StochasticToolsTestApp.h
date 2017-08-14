#ifndef STOCHASTICTOOLSTESTAPP_H
#define STOCHASTICTOOLSTESTAPP_H

#include "MooseApp.h"

class StochasticToolsTestApp;

template <>
InputParameters validParams<StochasticToolsTestApp>();

class StochasticToolsTestApp : public MooseApp
{
public:
  StochasticToolsTestApp(InputParameters parameters);
  virtual ~StochasticToolsTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* STOCHASTICTOOLSTESTAPP_H */
