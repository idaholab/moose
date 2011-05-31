#include "Elk.h"
#include "Factory.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"

// misc
#include "BodyForceRZ.h"
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
#include "ImposedVelocityBC.h"
#include "TemperatureAux.h"
#include "Temperature.h"
#include "SpecificHeatConstantVolumeAux.h"
#include "NSPressureAux.h"
#include "NSMomentumInviscidFluxAux.h"
#include "NodalMomentumInviscidFlux.h"
#include "NSEnthalpyAux.h"

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
#include "PLC_LSH.h"
#include "PowerLawCreepMaterial.h"
#include "PowerLawCreep.h"
#include "PlenumPressure.h"
#include "PlenumPressureRZ.h"
#include "Pressure.h"
#include "PressureRZ.h"
#include "PLSHPlasticMaterial.h"
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
  registerKernel(MassInviscidFlux);
  registerKernel(MomentumInviscidFlux);
  registerKernel(MomentumViscousFlux);
  registerKernel(EnergyInviscidFlux);
  registerKernel(EnergyViscousFlux);
  registerKernel(GravityPower);
  registerKernel(GravityForce);
  registerKernel(Temperature);
  registerBoundaryCondition(PressureNeumannBC);
  registerBoundaryCondition(ThermalBC);
  registerAux(VelocityAux);
  registerBoundaryCondition(ImposedVelocityBC);
  registerAux(TemperatureAux);
  registerAux(SpecificHeatConstantVolumeAux);
  registerAux(NSPressureAux);
  registerAux(NSMomentumInviscidFluxAux);
  registerKernel(NodalMomentumInviscidFlux);
  registerAux(NSEnthalpyAux);

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
  registerMaterial(PLC_LSH);
  registerMaterial(PLSHPlasticMaterial);
  registerMaterial(PowerLawCreepMaterial);
  registerMaterial(PowerLawCreep);
  registerBoundaryCondition(PlenumPressure);
  registerBoundaryCondition(PlenumPressureRZ);
  registerBoundaryCondition(Pressure);
  registerBoundaryCondition(PressureRZ);
  registerAux(ElasticEnergyAux);
  registerKernel(StressDivergence);
  registerKernel(StressDivergenceRZ);

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
  registerAction(ContactAction, "Contact/*", "add_dirac_kernel");
  registerDiracKernel(ContactMaster);
  registerDiracKernel(SlaveConstraint);

  // thermal contact
  registerAction(ThermalContactAction, "ThermalContact/*", "meta_action");

  // heat_conduction
  // This registers an action to add the "slave_flux" vector to the system at the right time
  registerActionName("add_slave_flux_vector", false);
  addActionNameDependency("add_slave_flux_vector", "ready_to_init");
  addActionNameDependency("init_problem", "add_slave_flux_vector");
  registerNonParsedAction(AddSlaveFluxVectorAction, "add_slave_flux_vector");

}
