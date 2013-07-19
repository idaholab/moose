#ifndef ELKAPP_H
#define ELKAPP_H

#include "MooseApp.h"

class ElkApp;

template<>
InputParameters validParams<ElkApp>();

class ElkApp : public MooseApp
{
public:
  ElkApp(const std::string & name, InputParameters parameters);
  virtual ~ElkApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* ELKAPP_H_ */
