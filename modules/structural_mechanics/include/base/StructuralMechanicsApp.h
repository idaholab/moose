#ifndef STRUCTURAL_MECHANICSAPP_H
#define STRUCTURAL_MECHANICSAPP_H

#include "MooseApp.h"

class StructuralMechanicsApp;

template<>
InputParameters validParams<StructuralMechanicsApp>();

class StructuralMechanicsApp : public MooseApp
{
public:
  StructuralMechanicsApp(InputParameters parameters);
  virtual ~StructuralMechanicsApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* STRUCTURAL_MECHANICSAPP_H */
