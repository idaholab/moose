#ifndef EXAMPLEAPP_H
#define EXAMPLEAPP_H

#include "MooseApp.h"

class ExampleApp;

template<>
InputParameters validParams<ExampleApp>();

class ExampleApp : public MooseApp
{
public:
  ExampleApp(const std::string & name, InputParameters parameters);
  virtual ~ExampleApp();
  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax& syntax, ActionFactory & action_factory);

};

#endif /* EXAMPLEAPP_H */
