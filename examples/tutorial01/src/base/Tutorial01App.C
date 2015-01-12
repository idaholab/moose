#include "Tutorial01App.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"

// InitialConditions
#include "ClosePackIC.h"

template<>
InputParameters validParams<Tutorial01App>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}

Tutorial01App::Tutorial01App(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);
  Tutorial01App::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
  Tutorial01App::associateSyntax(_syntax, _action_factory);
}

Tutorial01App::~Tutorial01App()
{
}

void
Tutorial01App::registerApps()
{
  registerApp(Tutorial01App);
}

void
Tutorial01App::registerObjects(Factory & factory)
{
  // InitialConditions
  registerInitialCondition(ClosePackIC);
}

void
Tutorial01App::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
}
