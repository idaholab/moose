#ifndef STORKAPP_H
#define STORKAPP_H

#include "MooseApp.h"

class StorkApp;

template<>
InputParameters validParams<StorkApp>();

class StorkApp : public MooseApp
{
public:
  StorkApp(InputParameters parameters);
  virtual ~StorkApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* STORKAPP_H */
