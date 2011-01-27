#include "Elk.h"
#include "MooseFactory.h"

// misc
#include "CoefDiffusion.h"
#include "Convection.h"
#include "InternalVolume.h"

// heat_conduction
#include "HeatConduction.h"
#include "HeatConductionImplicitEuler.h"
#include "HeatConductionMaterial.h"
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
#include "CLSHPlasticMaterial.h"
#include "CreepStrainAux.h"
#include "Elastic.h"
#include "Gravity.h"
#include "LinearIsotropicMaterial.h"
#include "LinearStrainHardening.h"
#include "LSHPlasticMaterial.h"
#include "PLC_LSH.h"
#include "PlasticMaterial.h"
#include "PlasticStrainAux.h"
#include "PowerLawCreepMaterial.h"
#include "PowerLawCreep.h"
#include "PlenumPressureBC.h"
#include "PressureBC.h"
#include "PLSHPlasticMaterial.h"
#include "StressAux.h"
#include "ElasticEnergyAux.h"
#include "StressDivergence.h"
#include "StressOutput.h"


// phase_field
#include "AC.h"
#include "ACBulk.h"
#include "ACInterface.h"
#include "CHBulk.h"
#include "CHInterface.h"
#include "CrossIC.h"
#include "SmoothCircleIC.h"
#include "RndSmoothCircleIC.h"
#include "RndBoundingBoxIC.h"
#include "GradientBoxIC.h"

// contact
#include "ContactMaster.h"
#include "SlaveConstraint.h"

void
Elk::registerObjects()
{
  // misc
  registerKernel(CoefDiffusion);
  registerKernel(Convection);
  registerPostprocessor(InternalVolume);

  // heat_conduction
  registerKernel(HeatConduction);
  registerKernel(HeatConductionImplicitEuler);
  registerNamedMaterial(HeatConductionMaterial, "HeatConduction");
  registerBoundaryCondition(FluxBC);

  // navier_stokes
  registerKernel(MassInviscidFlux);
  registerKernel(MomentumInviscidFlux);
  registerKernel(MomentumViscousFlux);
  registerKernel(EnergyInviscidFlux);
  registerKernel(EnergyViscousFlux);
  registerKernel(GravityPower);
  registerKernel(GravityForce);
  registerBoundaryCondition(PressureNeumannBC);
  registerBoundaryCondition(ThermalBC);
  registerAux(VelocityAux);

  // linear_elasticity
  registerKernel(SolidMechX);
  registerKernel(SolidMechY);
  registerKernel(SolidMechZ);
  registerKernel(SolidMechImplicitEuler);
  registerKernel(SolidMechTempCoupleX);
  registerKernel(SolidMechTempCoupleY);
  registerKernel(SolidMechTempCoupleZ);

  // solid_mechanics
  registerMaterial(CLSHPlasticMaterial);
  registerAux(CreepStrainAux);
  registerMaterial(Elastic);
  registerKernel(Gravity);
  registerNamedMaterial(LinearIsotropicMaterial, "LinearIsotropic");
  registerMaterial(LinearStrainHardening);
  registerMaterial(LSHPlasticMaterial);
  registerMaterial(PlasticMaterial);
  registerAux(PlasticStrainAux);
  registerMaterial(PLC_LSH);
  registerMaterial(PLSHPlasticMaterial);
  registerMaterial(PowerLawCreepMaterial);
  registerMaterial(PowerLawCreep);
  registerBoundaryCondition(PlenumPressureBC);
  registerBoundaryCondition(PressureBC);
  registerAux(StressAux);
  registerAux(ElasticEnergyAux);
  registerKernel(StressDivergence);
  registerKernel(StressOutput);

  // phase_field
  registerKernel(AC);
  registerKernel(ACBulk);
  registerKernel(ACInterface);
  registerKernel(CHBulk);
  registerKernel(CHInterface);
  registerInitialCondition(CrossIC);
  registerInitialCondition(SmoothCircleIC);
  registerInitialCondition(RndSmoothCircleIC);
  registerInitialCondition(RndBoundingBoxIC);
  registerInitialCondition(GradientBoxIC);

  // contact
  registerDiracKernel(ContactMaster);
  registerDiracKernel(SlaveConstraint);
}
