#ifndef POROUS_FLOWAPP_H
#define POROUS_FLOWAPP_H

#include "MooseApp.h"

class PorousFlowApp;

template <>
InputParameters validParams<PorousFlowApp>();

class PorousFlowApp : public MooseApp
{
public:
  PorousFlowApp(const InputParameters & parameters);
  virtual ~PorousFlowApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* POROUS_FLOWAPP_H */
