#ifndef MISCAPP_H
#define MISCAPP_H

#include "MooseApp.h"

class MiscApp;

template<>
InputParameters validParams<MiscApp>();

class MiscApp : public MooseApp
{
public:
  MiscApp(const std::string & name, InputParameters parameters);
  virtual ~MiscApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* MISCAPP_H */
