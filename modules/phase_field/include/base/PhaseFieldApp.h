#ifndef PHASE_FIELDAPP_H
#define PHASE_FIELDAPP_H

#include "MooseApp.h"

class PhaseFieldApp;

template<>
InputParameters validParams<PhaseFieldApp>();

class PhaseFieldApp : public MooseApp
{
public:
  PhaseFieldApp(const std::string & name, InputParameters parameters);
  virtual ~PhaseFieldApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* PHASE_FIELDAPP_H */
