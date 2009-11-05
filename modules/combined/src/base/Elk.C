#include "Elk.h"

#include "KernelFactory.h"
#include "BCFactory.h"
#include "AuxFactory.h"
#include "MaterialFactory.h"

// misc
#include "CoefDiffusion.h"

// heat_conduction
#include "HeatConduction.h"
#include "HeatConductionImplicitEuler.h"

// navier_stokes
#include "MassInviscidFlux.h"
#include "MomentumInviscidFlux.h"
#include "MomentumViscousFlux.h"
#include "EnergyInviscidFlux.h"
#include "EnergyViscousFlux.h"
#include "GravityPower.h"
#include "GravityForce.h"
#include "PressureNeumannBC.h"
#include "ThermalBC.h"
#include "VelocityAux.h"

void
Elk::registerObjects()
{
  // misc
  KernelFactory::instance()->registerKernel<CoefDiffusion>("CoefDiffusion");

  // heat_conduction
  KernelFactory::instance()->registerKernel<HeatConduction>("HeatConduction");
  KernelFactory::instance()->registerKernel<HeatConductionImplicitEuler>("HeatConductionImplicitEuler");

  // navier_stokes
  KernelFactory::instance()->registerKernel<MassInviscidFlux>("MassInviscidFlux");
  KernelFactory::instance()->registerKernel<MomentumInviscidFlux>("MomentumInviscidFlux");
  KernelFactory::instance()->registerKernel<MomentumViscousFlux>("MomentumViscousFlux");
  KernelFactory::instance()->registerKernel<EnergyInviscidFlux>("EnergyInviscidFlux");
  KernelFactory::instance()->registerKernel<EnergyViscousFlux>("EnergyViscousFlux");
  KernelFactory::instance()->registerKernel<GravityPower>("GravityPower");
  KernelFactory::instance()->registerKernel<GravityForce>("GravityForce");
  BCFactory::instance()->registerBC<PressureNeumannBC>("PressureNeumannBC");
  BCFactory::instance()->registerBC<ThermalBC>("ThermalBC");
  AuxFactory::instance()->registerAux<VelocityAux>("VelocityAux");
}
