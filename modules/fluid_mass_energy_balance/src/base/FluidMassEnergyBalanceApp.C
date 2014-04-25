#include "FluidMassEnergyBalanceApp.h"
#include "Moose.h"
#include "AppFactory.h"

////////////////////////////////////////////////////////////////
///      Souce and Sink, volume avagerged                     //
////////////////////////////////////////////////////////////////
#include "SourceSink.h"
#include "EnergyExtraction.h"

#include "EnthalpyTimeDerivative.h"
#include "EnthalpyImplicitEuler.h"
#include "EnthalpyDiffusion.h"
#include "EnthalpyConvectionWater.h"
#include "EnthalpyConvectionSteam.h"

///////////////////////////////////////////////////////////////
////    Single phase isothermal formulation: pressure        //
///////////////////////////////////////////////////////////////
#include "FluidFluxPressure.h"

//////////////////////////////////////////////////////////////
//       Two phase formulation: pressure & enthalpy         //
//////////////////////////////////////////////////////////////
#include "MassFluxTimeDerivative.h"
#include "WaterMassFluxPressure.h"
#include "SteamMassFluxPressure.h"
#include "WaterMassFluxElevation.h"

template<>
InputParameters validParams<FluidMassEnergyBalanceApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

FluidMassEnergyBalanceApp::FluidMassEnergyBalanceApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  FluidMassEnergyBalanceApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  FluidMassEnergyBalanceApp::associateSyntax(_syntax, _action_factory);
}

FluidMassEnergyBalanceApp::~FluidMassEnergyBalanceApp()
{
}

void
FluidMassEnergyBalanceApp::registerApps()
{
  registerApp(FluidMassEnergyBalanceApp);
}

void
FluidMassEnergyBalanceApp::registerObjects(Factory & factory)
{
  //energy
  registerKernel(EnthalpyImplicitEuler);
  registerKernel(EnthalpyTimeDerivative);
  registerKernel(EnthalpyDiffusion);
  registerKernel(EnthalpyConvectionWater);
  registerKernel(EnthalpyConvectionSteam);

  //source sink
  registerKernel(SourceSink);
  registerKernel(EnergyExtraction);

  //fluid-mass flow-two phase formulation
  registerKernel(MassFluxTimeDerivative);
  registerKernel(WaterMassFluxPressure);
  registerKernel(WaterMassFluxElevation);
  registerKernel(SteamMassFluxPressure);

  //isothermal flow for pressure field
  registerKernel(FluidFluxPressure);
}

void
FluidMassEnergyBalanceApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
