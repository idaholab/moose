#ifndef MODULESAPP_H
#define MODULESAPP_H

#include "MooseApp.h"

class ModulesApp;

template<>
InputParameters validParams<ModulesApp>();

class ModulesApp : public MooseApp
{
public:
  ModulesApp(const std::string & name, InputParameters parameters);
  virtual ~ModulesApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* MODULESAPP_H_ */
