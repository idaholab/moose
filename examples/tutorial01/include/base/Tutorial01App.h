#ifndef TUTORIAL01APP_H
#define TUTORIAL01APP_H

#include "MooseApp.h"

class Tutorial01App;

template<>
InputParameters validParams<Tutorial01App>();

class Tutorial01App : public MooseApp
{
public:
  Tutorial01App(const std::string & name, InputParameters parameters);
  virtual ~Tutorial01App();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* TUTORIAL01APP_H */
