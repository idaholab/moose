#ifndef POROUS_MEDIAAPP_H
#define POROUS_MEDIAAPP_H

#include "MooseApp.h"

class PorousMediaApp;

template<>
InputParameters validParams<PorousMediaApp>();

class PorousMediaApp : public MooseApp
{
public:
  PorousMediaApp(const std::string & name, InputParameters parameters);
  virtual ~PorousMediaApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* POROUS_MEDIAAPP_H */
