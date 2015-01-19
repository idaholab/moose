#include "DarcyThermoMechApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"

// Kernels
#include "DarcyPressure.h"

// Materials
#include "PackedColumn.h"

// AuxKernels
#include "DarcyVelocity.h"

template<>
InputParameters validParams<DarcyThermoMechApp>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}

DarcyThermoMechApp::DarcyThermoMechApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);
  DarcyThermoMechApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
  DarcyThermoMechApp::associateSyntax(_syntax, _action_factory);
}

DarcyThermoMechApp::~DarcyThermoMechApp()
{
}

void
DarcyThermoMechApp::registerApps()
{
  registerApp(DarcyThermoMechApp);
}

void
DarcyThermoMechApp::registerObjects(Factory & factory)
{
  registerKernel(DarcyPressure);
  registerMaterial(PackedColumn);
  registerAux(DarcyVelocity);
}

void
DarcyThermoMechApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
}
