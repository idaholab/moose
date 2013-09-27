#include "ElkApp.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AppFactory.h"

#include "SolidMechanicsModule.h"
#include "TensorMechanicsModule.h"
#include "PhaseFieldModule.h"
#include "ContactModule.h"
#include "HeatConductionModule.h"
#include "NavierStokesModule.h"
#include "LinearElasticityModule.h"
#include "FluidMassEnergyBalanceModule.h"
#include "ChemicalReactionsModule.h"
#include "MiscModule.h"
#include "VpscModule.h"


template<>
InputParameters validParams<ElkApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ElkApp::ElkApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  Moose::registerObjects(_factory);
  ElkApp::registerObjects(_factory);
  
  Moose::associateSyntax(_syntax, _action_factory);
  ElkApp::associateSyntax(_syntax, _action_factory);
}

ElkApp::~ElkApp()
{
}

void
ElkApp::registerApps()
{
  registerApp(ElkApp);
}

void
ElkApp::registerObjects(Factory & factory)
{
  Elk::SolidMechanics::registerObjects(factory);
  Elk::TensorMechanics::registerObjects(factory);
  Elk::PhaseField::registerObjects(factory);
  Elk::Contact::registerObjects(factory);
  Elk::HeatConduction::registerObjects(factory);
  Elk::NavierStokes::registerObjects(factory);
  Elk::LinearElasticity::registerObjects(factory);
  Elk::FluidMassEnergyBalance::registerObjects(factory);
  Elk::ChemicalReactions::registerObjects(factory);
  Elk::Misc::registerObjects(factory);
  Elk::Vpsc::registerObjects(factory);
}

void
ElkApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Elk::SolidMechanics::associateSyntax(syntax, action_factory);
  Elk::TensorMechanics::associateSyntax(syntax, action_factory);
  Elk::PhaseField::associateSyntax(syntax, action_factory);
  Elk::Contact::associateSyntax(syntax, action_factory);
  Elk::HeatConduction::associateSyntax(syntax, action_factory);
  Elk::ChemicalReactions::associateSyntax(syntax, action_factory);
  Elk::Misc::associateSyntax(syntax, action_factory);
  Elk::Vpsc::associateSyntax(syntax, action_factory);
}

