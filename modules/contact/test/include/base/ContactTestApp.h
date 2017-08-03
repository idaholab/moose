#ifndef CONTACTTESTAPP_H
#define CONTACTTESTAPP_H

#include "ContactApp.h"

class ContactTestApp;

template <>
InputParameters validParams<ContactTestApp>();

class ContactTestApp : public ContactApp
{
public:
  ContactTestApp(InputParameters parameters);
  virtual ~ContactTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* CONTACTTESTAPP_H */
