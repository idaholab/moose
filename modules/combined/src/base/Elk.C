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
#include "GravityRZ.h"
#include "LinearIsotropicMaterial.h"
#include "LinearIsotropicMaterialRZ.h"
#include "LinearStrainHardening.h"
#include "LSHPlasticMaterial.h"
#include "PLC_LSH.h"
#include "PlasticStrainAux.h"
#include "PowerLawCreepMaterial.h"
#include "PowerLawCreep.h"
#include "PlenumPressureBC.h"
#include "Pressure.h"
#include "PressureRZ.h"
#include "PLSHPlasticMaterial.h"
#include "StressAux.h"
#include "VelocityGradientAux.h"
#include "ElasticEnergyAux.h"
#include "StressDivergence.h"
#include "StressDivergenceRZ.h"
#include "StressOutput.h"


// phase_field
#include "AC.h"
#include "ACBulk.h"
#include "ACInterface.h"
#include "CHBulk.h"
#include "CHSplit1.h"
#include "CHSplit2LaPl.h"
#include "CHSplit2ChemPot.h"
#include "CHInterface.h"
#include "CrossIC.h"
#include "SmoothCircleIC.h"
#include "RndSmoothCircleIC.h"
#include "RndBoundingBoxIC.h"

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
  registerKernel(GravityRZ);
  registerNamedMaterial(LinearIsotropicMaterial, "LinearIsotropic");
  registerNamedMaterial(LinearIsotropicMaterialRZ, "LinearIsotropicRZ");
  registerMaterial(LinearStrainHardening);
  registerMaterial(LSHPlasticMaterial);
  registerAux(PlasticStrainAux);
  registerMaterial(PLC_LSH);
  registerMaterial(PLSHPlasticMaterial);
  registerMaterial(PowerLawCreepMaterial);
  registerMaterial(PowerLawCreep);
  registerBoundaryCondition(PlenumPressureBC);
  registerBoundaryCondition(Pressure);
  registerBoundaryCondition(PressureRZ);
  registerAux(StressAux);
  registerAux(VelocityGradientAux);
  registerAux(ElasticEnergyAux);
  registerKernel(StressDivergence);
  registerKernel(StressDivergenceRZ);
  registerKernel(StressOutput);

  // phase_field
  registerKernel(AC);
  registerKernel(ACBulk);
  registerKernel(ACInterface);
  registerKernel(CHBulk);
  registerKernel(CHSplit1);
  registerKernel(CHSplit2LaPl);
  registerKernel(CHSplit2ChemPot);
  registerKernel(CHInterface);
  registerInitialCondition(CrossIC);
  registerInitialCondition(SmoothCircleIC);
  registerInitialCondition(RndSmoothCircleIC);
  registerInitialCondition(RndBoundingBoxIC);

  // contact
  registerDiracKernel(ContactMaster);
  registerDiracKernel(SlaveConstraint);
}
