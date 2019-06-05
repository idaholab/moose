#include "THMTestApp.h"
#include "THMApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

#include "HeatConductionApp.h"
#include "MiscApp.h"
#include "FluidPropertiesApp.h"
#include "FluidPropertiesTestApp.h"

template <>
InputParameters
validParams<THMTestApp>()
{
  InputParameters params = validParams<THMApp>();
  return params;
}

THMTestApp::THMTestApp(InputParameters parameters) : THMApp(parameters)
{
  srand(processor_id());
  THMTestApp::registerAll(_factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

THMTestApp::~THMTestApp() {}

void
THMTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  THMApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"THMTestApp"});
    Registry::registerActionsTo(af, {"THMTestApp"});

    s.registerActionSyntax("JacobianTest1PhaseAction", "JacobianTest1Phase");
    s.registerActionSyntax("JacobianTest2PhaseAction", "JacobianTest2Phase");
    s.registerActionSyntax("JacobianTest2PhaseNCGAction", "JacobianTest2PhaseNCG");
    s.registerActionSyntax("JacobianTestGeneralAction", "JacobianTestGeneral");
    s.registerActionSyntax("JacobianTest1PhaseRDGAction", "JacobianTest1PhaseRDG");
    s.registerActionSyntax("JacobianTest2PhaseRDGBaseAction", "JacobianTest2PhaseRDG");
    s.registerActionSyntax("JacobianTest2PhaseNumericalFluxAction",
                           "JacobianTest2PhaseNumericalFlux");
    s.registerActionSyntax("JacobianTest2PhaseBoundaryFluxAction",
                           "JacobianTest2PhaseBoundaryFlux");
    s.registerActionSyntax("JacobianTest2PhaseRDGInterfacialVariablesAction",
                           "JacobianTest2PhaseRDGInterfacialVariables");
    s.registerActionSyntax("JacobianTest2PhaseRiemannSolverAction",
                           "JacobianTest2PhaseRiemannSolver");
    s.registerActionSyntax("JacobianTest2PhaseWaveSpeedsAction", "JacobianTest2PhaseWaveSpeeds");
    s.registerActionSyntax("ClosureTest1PhaseAction", "ClosureTest1Phase");
    s.registerActionSyntax("ClosureTest2PhaseAction", "ClosureTest2Phase");
  }
  HeatConductionApp::registerAll(f, af, s);
  FluidPropertiesApp::registerAll(f, af, s);
  THMApp::registerAll(f, af, s);
}

void
THMTestApp::registerApps()
{
  registerApp(THMApp);
  registerApp(THMTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
THMTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  THMTestApp::registerAll(f, af, s);
}

extern "C" void
THMTestApp__registerApps()
{
  THMTestApp::registerApps();
}
