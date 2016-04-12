#include "StructuralMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "StressDivergenceTruss.h"
#include "MaterialTruss.h"

template<>
InputParameters validParams<StructuralMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  params.set<bool>("use_legacy_output_syntax") = false;

  return params;
}

StructuralMechanicsApp::StructuralMechanicsApp(InputParameters parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  StructuralMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  StructuralMechanicsApp::associateSyntax(_syntax, _action_factory);
}

StructuralMechanicsApp::~StructuralMechanicsApp()
{
}

// External entry point for dynamic application loading
extern "C" void StructuralMechanicsApp__registerApps() { StructuralMechanicsApp::registerApps(); }
void
StructuralMechanicsApp::registerApps()
{
  registerApp(StructuralMechanicsApp);
}

// External entry point for dynamic object registration
extern "C" void StructuralMechanicsApp__registerObjects(Factory & factory) { StructuralMechanicsApp::registerObjects(factory); }
void
StructuralMechanicsApp::registerObjects(Factory & factory)
{
  registerMaterial(MaterialTruss);

  registerKernel(StressDivergenceTruss);
}

// External entry point for dynamic syntax association
extern "C" void StructuralMechanicsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { StructuralMechanicsApp::associateSyntax(syntax, action_factory); }
void
StructuralMechanicsApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
