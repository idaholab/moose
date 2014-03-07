#ifndef RICHARDSAPP_H
#define RICHARDSAPP_H

#include "MooseApp.h"

class RichardsApp;

template<>
InputParameters validParams<RichardsApp>();

class RichardsApp : public MooseApp
{
public:
  RichardsApp(const std::string & name, InputParameters parameters);
  virtual ~RichardsApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* RICHARDSAPP_H */
