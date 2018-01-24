#ifndef MISCTESTAPP_H
#define MISCTESTAPP_H

#include "MooseApp.h"

class MiscTestApp;

template <>
InputParameters validParams<MiscTestApp>();

class MiscTestApp : public MooseApp
{
public:
  MiscTestApp(InputParameters parameters);
  virtual ~MiscTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* MISCTESTAPP_H */
