#include "Elk.h"
#include "Factory.h"
#include "ActionFactory.h"

// misc
#include "BodyForceRZ.h"
#include "BodyForceVoid.h"
#include "CoefDiffusion.h"
#include "Convection.h"
#include "InternalVolume.h"
#include "InternalVolumeRZ.h"
#include "NeumannRZ.h"
#include "SideAverageValueRZ.h"
#include "SideFluxIntegralRZ.h"
#include "SideIntegralRZ.h"

// heat_conduction
#include "AddSlaveFluxVectorAction.h"
#include "ConvectiveFluxRZ.h"
#include "GapHeatPointSourceMaster.h"
#include "GapHeatTransfer.h"
#include "GapHeatTransferRZ.h"
#include "HeatConduction.h"
#include "HeatConductionRZ.h"
#include "HeatConductionImplicitEuler.h"
#include "HeatConductionImplicitEulerRZ.h"
#include "HeatConductionMaterial.h"


// navier_stokes
#include "NSMassInviscidFlux.h"
#include "NSMomentumInviscidFlux.h"
#include "NSEnergyInviscidFlux.h"
#include "NSGravityPower.h"
#include "NSGravityForce.h"
#include "NSPressureNeumannBC.h"
#include "NSThermalBC.h"
#include "NSVelocityAux.h"
#include "NSImposedVelocityBC.h"
#include "NSTemperatureAux.h"
#include "NSTemperatureL2.h"
#include "NSPressureAux.h"
#include "NSEnthalpyAux.h"
#include "NSEnergyThermalFlux.h"
#include "NSMomentumViscousFlux.h"
#include "NSEnergyViscousFlux.h"
#include "NSMomentumInviscidFluxWithGradP.h"
#include "NSSUPGMomentum.h"
#include "NSSUPGMass.h"
#include "NSSUPGEnergy.h"
#include "NSMomentumBC.h"
#include "NSEnergyBC.h"
#include "NSMassBC.h"
#include "NSInflowThermalBC.h"

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
#include "Elastic.h"
#include "ElasticEnergyAux.h"
#include "Gravity.h"
#include "GravityRZ.h"
#include "LinearAnisotropicMaterial.h"
#include "LinearIsotropicMaterial.h"
#include "LinearIsotropicMaterialRZ.h"
#include "LinearStrainHardening.h"
#include "LSHPlasticMaterial.h"
#include "LSHPlasticMaterialRZ.h"
#include "MaterialTensorAux.h"
#include "MaterialVectorAux.h"
#include "PLC_LSH.h"
#include "PowerLawCreepMaterial.h"
#include "PowerLawCreep.h"
#include "PlenumPressure.h"
#include "PlenumPressureAction.h"
#include "PlenumPressureRZ.h"
#include "PlenumPressureRZAction.h"
#include "Pressure.h"
#include "PressureAction.h"
#include "PressureRZ.h"
#include "PressureRZAction.h"
#include "PLSHPlasticMaterial.h"
#include "SolidMechanicsAction.h"
#include "StressDivergence.h"
#include "StressDivergenceRZ.h"


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
#include "ContactAction.h"
#include "ContactMaster.h"
#include "SlaveConstraint.h"

// thermal contact
#include "ThermalContactAction.h"


void
Elk::registerObjects()
{
  // misc
  registerKernel(BodyForceRZ);
  registerKernel(BodyForceVoid);
  registerKernel(CoefDiffusion);
  registerKernel(Convection);
  registerPostprocessor(InternalVolume);
  registerPostprocessor(InternalVolumeRZ);
  registerBoundaryCondition(NeumannRZ);
  registerPostprocessor(SideAverageValueRZ);
  registerPostprocessor(SideFluxIntegralRZ);
  registerPostprocessor(SideIntegralRZ);

  registerBoundaryCondition(ConvectiveFluxRZ);
  registerBoundaryCondition(GapHeatTransfer);
  registerBoundaryCondition(GapHeatTransferRZ);
  registerKernel(HeatConduction);
  registerKernel(HeatConductionRZ);
  registerKernel(HeatConductionImplicitEuler);
  registerKernel(HeatConductionImplicitEulerRZ);
  registerMaterial(HeatConductionMaterial);
  registerDiracKernel(GapHeatPointSourceMaster);

  // navier_stokes
  registerKernel(NSMassInviscidFlux);
  registerKernel(NSMomentumInviscidFlux);
  registerKernel(NSEnergyInviscidFlux);
  registerKernel(NSGravityPower);
  registerKernel(NSGravityForce);
  registerKernel(NSTemperatureL2);
  registerBoundaryCondition(NSPressureNeumannBC);
  registerBoundaryCondition(NSThermalBC);
  registerAux(NSVelocityAux);
  registerBoundaryCondition(NSImposedVelocityBC);
  registerAux(NSTemperatureAux);
  registerAux(NSPressureAux);
  registerAux(NSEnthalpyAux);
  registerKernel(NSEnergyThermalFlux);
  registerKernel(NSMomentumViscousFlux);
  registerKernel(NSEnergyViscousFlux);
  registerKernel(NSMomentumInviscidFluxWithGradP);
  registerKernel(NSSUPGMomentum);
  registerKernel(NSSUPGMass);
  registerKernel(NSSUPGEnergy);
  registerBoundaryCondition(NSMomentumBC);
  registerBoundaryCondition(NSEnergyBC);
  registerBoundaryCondition(NSMassBC);
  registerBoundaryCondition(NSInflowThermalBC);

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
  registerMaterial(Elastic);
  registerKernel(Gravity);
  registerKernel(GravityRZ);
  registerMaterial(LinearAnisotropicMaterial);
  registerMaterial(LinearIsotropicMaterial);
  registerMaterial(LinearIsotropicMaterialRZ);
  registerMaterial(LinearStrainHardening);
  registerMaterial(LSHPlasticMaterial);
  registerMaterial(LSHPlasticMaterialRZ);
  registerAux(MaterialTensorAux);
  registerAux(MaterialVectorAux);
  registerMaterial(PLC_LSH);
  registerMaterial(PLSHPlasticMaterial);
  registerMaterial(PowerLawCreepMaterial);
  registerMaterial(PowerLawCreep);

  registerBoundaryCondition(PlenumPressure);
  registerAction(PlenumPressureAction, "add_bc");

  registerBoundaryCondition(PlenumPressureRZ);
  registerAction(PlenumPressureRZAction, "add_bc");

  registerBoundaryCondition(Pressure);
  registerAction(PressureAction, "add_bc");

  registerBoundaryCondition(PressureRZ);
  registerAction(PressureRZAction, "add_bc");

  registerAux(ElasticEnergyAux);
  registerKernel(StressDivergence);
  registerKernel(StressDivergenceRZ);
  registerAction(SolidMechanicsAction, "add_kernel");

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
  registerAction(ContactAction, "meta_action");
  registerDiracKernel(ContactMaster);
  registerDiracKernel(SlaveConstraint);

  // thermal contact
  registerAction(ThermalContactAction, "meta_action");

  // heat_conduction
  // This registers an action to add the "slave_flux" vector to the system at the right time
  registerActionName("add_slave_flux_vector", false);
  addActionNameDependency("add_slave_flux_vector", "ready_to_init");
  addActionNameDependency("init_problem", "add_slave_flux_vector");
  registerAction(AddSlaveFluxVectorAction, "add_slave_flux_vector");

}
