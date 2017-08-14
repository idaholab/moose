#ifndef CONTACTTESTAPP_H
#define CONTACTTESTAPP_H

#include "MooseApp.h"

class ContactTestApp;

template <>
InputParameters validParams<ContactTestApp>();

class ContactTestApp : public MooseApp
{
public:
  ContactTestApp(InputParameters parameters);
  virtual ~ContactTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* CONTACTTESTAPP_H */
