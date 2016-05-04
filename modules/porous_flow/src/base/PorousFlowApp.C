#include "PorousFlowApp.h"
#include "Moose.h"
#include "TensorMechanicsApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

// UserObjects
#include "PorousFlowDictator.h"

// Postprocessors
#include "PorousFlowFluidMass.h"

template<>
InputParameters validParams<PorousFlowApp>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  params.set<bool>("use_legacy_output_syntax") = false;

  return params;
}

PorousFlowApp::PorousFlowApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  TensorMechanicsApp::registerObjects(_factory);
  PorousFlowApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  TensorMechanicsApp::associateSyntax(_syntax, _action_factory);
  PorousFlowApp::associateSyntax(_syntax, _action_factory);
}

PorousFlowApp::~PorousFlowApp()
{
}

// External entry point for dynamic application loading
extern "C" void PorousFlowApp__registerApps() { PorousFlowApp::registerApps(); }
void
PorousFlowApp::registerApps()
{
  registerApp(PorousFlowApp);
}

// External entry point for dynamic object registration
extern "C" void PorousFlowApp__registerObjects(Factory & factory) { PorousFlowApp::registerObjects(factory); }
void
PorousFlowApp::registerObjects(Factory & factory)
{
  // UserObjects
  registerUserObject(PorousFlowDictator);

  // Postprocessors
  registerPostprocessor(PorousFlowFluidMass);
}

// External entry point for dynamic syntax association
extern "C" void PorousFlowApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { PorousFlowApp::associateSyntax(syntax, action_factory); }
void
PorousFlowApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
