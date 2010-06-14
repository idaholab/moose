#include "Elk.h"

#include "KernelFactory.h"
#include "BCFactory.h"
#include "AuxFactory.h"
#include "MaterialFactory.h"

// misc
#include "CoefDiffusion.h"
#include "Convection.h"

// heat_conduction
#include "HeatConduction.h"
#include "HeatConductionImplicitEuler.h"
#include "FluxBC.h"

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

// linear_elasticity
#include "SolidMechX.h"
#include "SolidMechY.h"
#include "SolidMechZ.h"
#include "SolidMechImplicitEuler.h"
#include "SolidMechTempCoupleX.h"
#include "SolidMechTempCoupleY.h"
#include "SolidMechTempCoupleZ.h"

// solid_mechanics
#include "StressDivergence.h"
#include "LinearIsotropicMaterial.h"
#include "PlasticMaterial.h"
#include "DeltaGamma.h"

void
Elk::registerObjects()
{
  // misc
  KernelFactory::instance()->registerKernel<CoefDiffusion>("CoefDiffusion");
  KernelFactory::instance()->registerKernel<Convection>("Convection");

  // heat_conduction
  KernelFactory::instance()->registerKernel<HeatConduction>("HeatConduction");
  KernelFactory::instance()->registerKernel<HeatConductionImplicitEuler>("HeatConductionImplicitEuler");
  BCFactory::instance()->registerBC<FluxBC>("FluxBC");

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

  // linear_elasticity
  KernelFactory::instance()->registerKernel<SolidMechX>("SolidMechX");
  KernelFactory::instance()->registerKernel<SolidMechY>("SolidMechY");
  KernelFactory::instance()->registerKernel<SolidMechZ>("SolidMechZ");
  KernelFactory::instance()->registerKernel<SolidMechImplicitEuler>("SolidMechImplicitEuler");
  KernelFactory::instance()->registerKernel<SolidMechTempCoupleX>("SolidMechTempCoupleX");
  KernelFactory::instance()->registerKernel<SolidMechTempCoupleY>("SolidMechTempCoupleY");
  KernelFactory::instance()->registerKernel<SolidMechTempCoupleZ>("SolidMechTempCoupleZ");

  // solid_mechanics
  KernelFactory::instance()->registerKernel<StressDivergence>("StressDivergence");
  KernelFactory::instance()->registerKernel<DeltaGamma>("DeltaGamma");
  MaterialFactory::instance()->registerMaterial<LinearIsotropicMaterial>("LinearIsotropic");
  MaterialFactory::instance()->registerMaterial<PlasticMaterial>("PlasticMaterial");
}
