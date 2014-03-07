#include "ModulesApp.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AppFactory.h"

/************************************************************
 * New Module Step 1.                                       *
 *         Add include for new modules here                 *
 * #if defined(MODULENAME) || defined(ALL_MODULES)          *
 *   #include "ModuleNameApp.h"                             *
 * #endif                                                   *
 ***********************************************************/
#if defined(CHEMICAL_REACTIONS) || defined(ALL_MODULES)
  #include "ChemicalReactionsApp.h"
#endif
#if defined(CONTACT) || defined(ALL_MODULES)
  #include "ContactApp.h"
#endif
#if defined(FLUID_MASS_ENERGY_BALANCE) || defined(ALL_MODULES)
  #include "FluidMassEnergyBalanceApp.h"
#endif
#if defined(HEAT_CONDUCTION) || defined(ALL_MODULES)
  #include "HeatConductionApp.h"
#endif
#if defined(LINEAR_ELASTICITY) || defined(ALL_MODULES)
  #include "LinearElasticityApp.h"
#endif
#if defined(MISC) || defined(ALL_MODULES)
  #include "MiscApp.h"
#endif
#if defined(NAVIER_STOKES) || defined(ALL_MODULES)
  #include "NavierStokesApp.h"
#endif
#if defined(PHASE_FIELD) || defined(ALL_MODULES)
  #include "PhaseFieldApp.h"
#endif
#if defined(RICHARDS) || defined(ALL_MODULES)
  #include "RichardsApp.h"
#endif
#if defined(SOLID_MECHANICS) || defined(ALL_MODULES)
  #include "SolidMechanicsApp.h"
#endif
#if defined(TENSOR_MECHANICS) || defined(ALL_MODULES)
  #include "TensorMechanicsApp.h"
#endif
#if defined(WATER_STEAM_EOS) || defined(ALL_MODULES)
  #include "WaterSteamEOSApp.h"
#endif

template<>
InputParameters validParams<ModulesApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ModulesApp::ModulesApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
}

ModulesApp::~ModulesApp()
{
}

void
ModulesApp::registerApps()
{
  registerApp(ModulesApp);
}

void
ModulesApp::registerObjects(Factory & factory)
{
  /************************************************************
   * New Module Step 2.                                       *
   *                Register module objects here              *
   * ModuleNameApp::registerObjects(factory);                 *
   * #if defined(MODULENAME) || defined(ALL_MODULES)          *
   *   ModulenameApp::registerObjects(factory);               *
   * #endif                                                   *
   ***********************************************************/
#if defined(CHEMICAL_REACTIONS) || defined(ALL_MODULES)
  ChemicalReactionsApp::registerObjects(factory);
#endif
#if defined(CONTACT) || defined(ALL_MODULES)
  ContactApp::registerObjects(factory);
#endif
#if defined(FLUID_MASS_ENERGY_BALANCE) || defined(ALL_MODULES)
  FluidMassEnergyBalanceApp::registerObjects(factory);
#endif
#if defined(HEAT_CONDUCTION) || defined(ALL_MODULES)
  HeatConductionApp::registerObjects(factory);
#endif
#if defined(LINEAR_ELASTICITY) || defined(ALL_MODULES)
  LinearElasticityApp::registerObjects(factory);
#endif
#if defined(MISC) || defined(ALL_MODULES)
  MiscApp::registerObjects(factory);
#endif
#if defined(NAVIER_STOKES) || defined(ALL_MODULES)
  NavierStokesApp::registerObjects(factory);
#endif
#if defined(PHASE_FIELD) || defined(ALL_MODULES)
  PhaseFieldApp::registerObjects(factory);
#endif
#if defined(RICHARDS) || defined(ALL_MODULES)
  RichardsApp::registerObjects(factory);
#endif
#if defined(SOLID_MECHANICS) || defined(ALL_MODULES)
  SolidMechanicsApp::registerObjects(factory);
#endif
#if defined(TENSOR_MECHANICS) || defined(ALL_MODULES)
  TensorMechanicsApp::registerObjects(factory);
#endif
#if defined(WATER_STEAM_EOS) || defined(ALL_MODULES)
  WaterSteamEOSApp::registerObjects(factory);
#endif
}

void
ModulesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  /************************************************************
   * New Module Step 3.                                       *
   *                Associate syntax here                     *
   * #if defined(MODULENAME) || defined(ALL_MODULES)          *
   *   ModuleNameApp::associateSyntax(syntax, action_factory);*
   * #endif                                                   *
   ***********************************************************/
#if defined(CHEMICAL_REACTIONS) || defined(ALL_MODULES)
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
#endif
#if defined(CONTACT) || defined(ALL_MODULES)
  ContactApp::associateSyntax(syntax, action_factory);
#endif
#if defined(FLUID_MASS_ENERGY_BALANCE) || defined(ALL_MODULES)
  FluidMassEnergyBalanceApp::associateSyntax(syntax, action_factory);
#endif
#if defined(HEAT_CONDUCTION) || defined(ALL_MODULES)
  HeatConductionApp::associateSyntax(syntax, action_factory);
#endif
#if defined(LINEAR_ELASTICITY) || defined(ALL_MODULES)
  LinearElasticityApp::associateSyntax(syntax, action_factory);
#endif
#if defined(MISC) || defined(ALL_MODULES)
  MiscApp::associateSyntax(syntax, action_factory);
#endif
#if defined(NAVIER_STOKES) || defined(ALL_MODULES)
  NavierStokesApp::associateSyntax(syntax, action_factory);
#endif
#if defined(PHASE_FIELD) || defined(ALL_MODULES)
  PhaseFieldApp::associateSyntax(syntax, action_factory);
#endif
#if defined(RICHARDS) || defined(ALL_MODULES)
  RichardsApp::associateSyntax(syntax, action_factory);
#endif
#if defined(SOLID_MECHANICS) || defined(ALL_MODULES)
  SolidMechanicsApp::associateSyntax(syntax, action_factory);
#endif
#if defined(TENSOR_MECHANICS) || defined(ALL_MODULES)
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
#endif
#if defined(WATER_STEAM_EOS) || defined(ALL_MODULES)
  WaterSteamEOSApp::associateSyntax(syntax, action_factory);
#endif
}
