#ifndef STOCHASTIC_TOOLSAPP_H
#define STOCHASTIC_TOOLSAPP_H

#include "MooseApp.h"

class StochasticToolsApp;

template <>
InputParameters validParams<StochasticToolsApp>();

class StochasticToolsApp : public MooseApp
{
public:
  StochasticToolsApp(InputParameters parameters);
  virtual ~StochasticToolsApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* STOCHASTIC_TOOLSAPP_H */
