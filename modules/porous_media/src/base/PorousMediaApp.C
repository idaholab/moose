#include "PorousMediaApp.h"
#include "Moose.h"
#include "AppFactory.h"

#include "ActionFactory.h"
#include "Syntax.h"

//kernels

///////////////////////////////////////////////////////////////
//      Single phase formulation: pressure & temperature     //
///////////////////////////////////////////////////////////////
#include "TemperatureTimeDerivative.h"
#include "TemperatureDiffusion.h"
#include "TemperatureConvection.h"

#include "MassFluxTimeDerivative_PT.h"
#include "MassFluxTimeDerivative_PT_comp.h"
#include "WaterMassFluxPressure_PT.h"
#include "WaterMassFluxElevation_PT.h"

#include "SourceSink.h"
//auxkernels
#include "VelocityAux.h"

//BCs
#include "OutFlowBC.h"

//materials
#include "PorousMedia.h"
#include "FluidFlow.h"
#include "HeatTransport.h"
#include "ChemicalReactions.h"
#include "Geothermal.h"

//userobjects
#include "WaterSteamEOS.h"

//actions
#include "GeothermalMaterialAction.h"


template<>
InputParameters validParams<PorousMediaApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

PorousMediaApp::PorousMediaApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  PorousMediaApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PorousMediaApp::associateSyntax(_syntax, _action_factory);
}

PorousMediaApp::~PorousMediaApp()
{
}

void
PorousMediaApp::registerApps()
{
  registerApp(PorousMediaApp);
}

void
PorousMediaApp::registerObjects(Factory & factory)
{

  //heat transport-PT formulation, single phase only
  registerKernel(TemperatureTimeDerivative);
  registerKernel(TemperatureDiffusion);
  registerKernel(TemperatureConvection);

  //fluid-mass flow-single phase formulation
  registerKernel(MassFluxTimeDerivative_PT);
  registerKernel(MassFluxTimeDerivative_PT_comp);
  registerKernel(WaterMassFluxPressure_PT);
  registerKernel(WaterMassFluxElevation_PT);

  registerKernel(SourceSink);
  //auxkernels
  registerAux(VelocityAux);

  //BCs
  registerBoundaryCondition(OutFlowBC);

  //materials
  registerMaterial(PorousMedia);
  registerMaterial(FluidFlow);
  registerMaterial(HeatTransport);
  registerMaterial(ChemicalReactions);
  registerMaterial(Geothermal);

  //userobjects
  registerUserObject(WaterSteamEOS);

}

void
PorousMediaApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerAction(GeothermalMaterialAction, "add_material");
  syntax.registerActionSyntax("GeothermalMaterialAction", "Materials/GeothermalMaterial");
}
