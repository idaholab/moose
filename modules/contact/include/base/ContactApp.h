#ifndef CONTACTAPP_H
#define CONTACTAPP_H

#include "MooseApp.h"

class ContactApp;

template<>
InputParameters validParams<ContactApp>();

class ContactApp : public MooseApp
{
public:
  ContactApp(const std::string & name, InputParameters parameters);
  virtual ~ContactApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* CONTACTAPP_H */
