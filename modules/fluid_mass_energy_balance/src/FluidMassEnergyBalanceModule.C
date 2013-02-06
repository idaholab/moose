#include "FluidMassEnergyBalanceModule.h"
#include "Factory.h"
#include "ActionFactory.h"

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


void
Elk::FluidMassEnergyBalance::registerObjects(Factory & factory)
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
