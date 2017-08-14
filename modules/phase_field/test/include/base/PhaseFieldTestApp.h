#ifndef PHASEFIELDTESTAPP_H
#define PHASEFIELDTESTAPP_H

#include "MooseApp.h"

class PhaseFieldTestApp;

template <>
InputParameters validParams<PhaseFieldTestApp>();

class PhaseFieldTestApp : public MooseApp
{
public:
  PhaseFieldTestApp(InputParameters parameters);
  virtual ~PhaseFieldTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* PHASEFIELDTESTAPP_H */
