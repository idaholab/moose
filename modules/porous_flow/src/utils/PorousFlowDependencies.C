//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDependencies.h"

PorousFlowDependencies::PorousFlowDependencies()
{
  _deps.insertDependency("PorousFlowAdvectiveFlux", "PorousFlowDarcyBase");
  _deps.insertDependency("PorousFlowAdvectiveFlux", "PorousFlowMassFraction_nodal");
  _deps.insertDependency("PorousFlowAdvectiveFlux", "PorousFlowRelativePermeability_nodal");
  _deps.insertDependency("PorousFlowDarcyBase", "PorousFlowPermeability_qp");
  _deps.insertDependency("PorousFlowDarcyBase", "PorousFlowDensity_qp");
  _deps.insertDependency("PorousFlowDarcyBase", "PorousFlowDensity_nodal");
  _deps.insertDependency("PorousFlowDarcyBase", "PorousFlowViscosity_nodal");
  _deps.insertDependency("PorousFlowDarcyBase", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowEffectiveStressCoupling",
                         "PorousFlowEffectiveFluidPressure_qp");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "PorousFlowPorosity_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "PorousFlowNearestQP_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "PorousFlowMatrixInternalEnergy_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "PorousFlowDensity_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowEnergyTimeDerivative", "PorousFlowInternalEnergy_nodal");
  _deps.insertDependency("PorousFlowFullySaturatedDarcyBase", "PorousFlowPermeability_qp");
  _deps.insertDependency("PorousFlowFullySaturatedDarcyBase", "PorousFlowDensity_qp");
  _deps.insertDependency("PorousFlowFullySaturatedDarcyBase", "PorousFlowViscosity_qp");
  _deps.insertDependency("PorousFlowFullySaturatedDarcyBase", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowFullySaturatedDarcyFlow", "PorousFlowFullySaturatedDarcyBase");
  _deps.insertDependency("PorousFlowFullySaturatedDarcyFlow", "PorousFlowMassFraction_qp");
  _deps.insertDependency("PorousFlowFullySaturatedHeatAdvection",
                         "PorousFlowFullySaturatedDarcyBase");
  _deps.insertDependency("PorousFlowFullySaturatedHeatAdvection", "PorousFlowEnthalpy_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative",
                         "PorousFlowConstantBiotModulus_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative",
                         "PorousFlowConstantThermalExpansionCoefficient_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative", "PorousFlowDensity_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative", "PorousFlowTemperature_qp");
  _deps.insertDependency("PorousFlowFullySaturatedMassTimeDerivative",
                         "PorousFlowVolumetricStrain_qp");
  _deps.insertDependency("PorousFlowHeatAdvection", "PorousFlowDarcyBase");
  _deps.insertDependency("PorousFlowHeatAdvection", "PorousFlowEnthalpy_nodal");
  _deps.insertDependency("PorousFlowHeatAdvection", "PorousFlowRelativePermeability_nodal");
  _deps.insertDependency("PorousFlowHeatConduction", "PorousFlowThermalConductivity_qp");
  _deps.insertDependency("PorousFlowHeatConduction", "PorousFlowTemperature_qp");
  _deps.insertDependency("PorousFlowHeatConduction", "PorousFlowTemperature_qp");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "PorousFlowPorosity_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "PorousFlowNearestQP_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion",
                         "PorousFlowMatrixInternalEnergy_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "PorousFlowDensity_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "PorousFlowInternalEnergy_nodal");
  _deps.insertDependency("PorousFlowHeatVolumetricExpansion", "PorousFlowVolumetricStrain_qp");
  _deps.insertDependency("PorousFlowMassRadioactiveDecay", "PorousFlowPorosity_nodal");
  _deps.insertDependency("PorousFlowMassRadioactiveDecay", "PorousFlowNearestQP_nodal");
  _deps.insertDependency("PorousFlowMassRadioactiveDecay", "PorousFlowDensity_nodal");
  _deps.insertDependency("PorousFlowMassRadioactiveDecay", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowMassRadioactiveDecay", "PorousFlowMassFraction_nodal");
  _deps.insertDependency("PorousFlowMassTimeDerivative", "PorousFlowPorosity_nodal");
  _deps.insertDependency("PorousFlowMassTimeDerivative", "PorousFlowNearestQP_nodal");
  _deps.insertDependency("PorousFlowMassTimeDerivative", "PorousFlowDensity_nodal");
  _deps.insertDependency("PorousFlowMassTimeDerivative", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowMassTimeDerivative", "PorousFlowMassFraction_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "PorousFlowPorosity_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "PorousFlowNearestQP_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "PorousFlowDensity_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "PorousFlowMassFraction_nodal");
  _deps.insertDependency("PorousFlowMassVolumetricExpansion", "PorousFlowVolumetricStrain_qp");
  _deps.insertDependency("PorousFlowPlasticHeatEnergy", "PlasticHeatEnergy");
  _deps.insertDependency("PorousFlowPlasticHeatEnergy", "PorousFlowNearestQP_nodal");
  _deps.insertDependency("PorousFlowPlasticHeatEnergy", "PorousFlowPorosity_nodal");
  _deps.insertDependency("PorousFlowDesorpedMassTimeDerivative", "PorousFlowPorosity_qp");
  _deps.insertDependency("PorousFlowDesorpedMassVolumetricExpansion", "PorousFlowPorosity_qp");
  _deps.insertDependency("PorousFlowDesorpedMassVolumetricExpansion",
                         "PorousFlowVolumetricStrain_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "PorousFlowDensity_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "PorousFlowMassFraction_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "PorousFlowPorosity_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "PorousFlowDiffusivity_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "PorousFlowDiffusionCoefficient_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "PorousFlowRelativePermeability_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "PorousFlowViscosity_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "PorousFlowPermeability_qp");
  _deps.insertDependency("PorousFlowDispersiveFlux", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowDarcyVelocityComponent", "PorousFlowRelativePermeability_qp");
  _deps.insertDependency("PorousFlowDarcyVelocityComponent", "PorousFlowViscosity_qp");
  _deps.insertDependency("PorousFlowDarcyVelocityComponent", "PorousFlowPermeability_qp");
  _deps.insertDependency("PorousFlowDarcyVelocityComponent", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowDarcyVelocityComponent", "PorousFlowDensity_qp");
  _deps.insertDependency("PressureAux", "PorousFlowPS_qp");
  _deps.insertDependency("SaturationAux", "PorousFlowPS_qp");
  _deps.insertDependency("TemperatureAux", "PorousFlowTemperature_qp");
  _deps.insertDependency("DensityAux", "PorousFlowDensity_qp");
  _deps.insertDependency("ViscosityAux", "PorousFlowViscosity_qp");
  _deps.insertDependency("MassFractionAux", "PorousFlowMassFraction_qp");
  _deps.insertDependency("RelativePermeabilityAux", "PorousFlowRelativePermeability_qp");
  _deps.insertDependency("EnthalpyAux", "PorousFlowEnthalpy_qp");
  _deps.insertDependency("InternalEnergyAux", "PorousFlowInternalEnergy_qp");
  _deps.insertDependency("PorousFlowConstantBiotModulus_nodal", "PorousFlowPorosity_nodal");
  _deps.insertDependency("PorousFlowConstantBiotModulus_qp", "PorousFlowPorosity_qp");
  _deps.insertDependency("PorousFlowConstantThermalExpansionCoefficient_nodal",
                         "PorousFlowPorosity_nodal");
  _deps.insertDependency("PorousFlowConstantThermalExpansionCoefficient_qp",
                         "PorousFlowPorosity_qp");
  _deps.insertDependency("PorousFlowDensity_nodal", "PorousFlowFluidPropertiesBase_nodal");
  _deps.insertDependency("PorousFlowDensity_qp", "PorousFlowFluidPropertiesBase_qp");
  _deps.insertDependency("PorousFlowFluidPropertiesBase_nodal", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowFluidPropertiesBase_nodal", "PorousFlowTemperature_nodal");
  _deps.insertDependency("PorousFlowFluidPropertiesBase_qp", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowFluidPropertiesBase_qp", "PorousFlowTemperature_qp");
  _deps.insertDependency("PorousFlowDiffusivity_nodal", "PorousFlowMaterialVectorBase_nodal");
  _deps.insertDependency("PorousFlowDiffusivity_nodal", "PorousFlowPorosity_nodal");
  _deps.insertDependency("PorousFlowDiffusivity_nodal", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowDiffusivity_qp", "PorousFlowMaterialVectorBase_qp");
  _deps.insertDependency("PorousFlowDiffusivity_qp", "PorousFlowPorosity_qp");
  _deps.insertDependency("PorousFlowDiffusivity_qp", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowEffectiveFluidPressure_nodal",
                         "PorousFlowMaterialVectorBase_nodal");
  _deps.insertDependency("PorousFlowEffectiveFluidPressure_nodal", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowEffectiveFluidPressure_qp", "PorousFlowMaterialVectorBase_qp");
  _deps.insertDependency("PorousFlowEffectiveFluidPressure_qp", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowEnthalpy_nodal", "PorousFlowFluidPropertiesBase_nodal");
  _deps.insertDependency("PorousFlowEnthalpy_nodal", "PorousFlowInternalEnergy_nodal");
  _deps.insertDependency("PorousFlowEnthalpy_nodal", "PorousFlowDensity_nodal");
  _deps.insertDependency("PorousFlowEnthalpy_qp", "PorousFlowFluidPropertiesBase_qp");
  _deps.insertDependency("PorousFlowEnthalpy_qp", "PorousFlowInternalEnergy_qp");
  _deps.insertDependency("PorousFlowEnthalpy_qp", "PorousFlowDensity_qp");
  _deps.insertDependency("PorousFlowFluidStateFlashBase_nodal", "PorousFlowVariableBase_nodal");
  _deps.insertDependency("PorousFlowFluidStateFlashBase_nodal", "PorousFlowTemperature_nodal");
  _deps.insertDependency("PorousFlowFluidStateFlashBase_nodal", "PorousFlowMassFraction_nodal");
  _deps.insertDependency("PorousFlowFluidStateFlashBase_nodal", "PorousFlowDensity_nodal");
  _deps.insertDependency("PorousFlowFluidStateFlashBase_nodal", "PorousFlowViscosity_nodal");
  _deps.insertDependency("PorousFlowFluidStateFlashBase_qp", "PorousFlowVariableBase_qp");
  _deps.insertDependency("PorousFlowFluidStateFlashBase_qp", "PorousFlowTemperature_qp");
  _deps.insertDependency("PorousFlowFluidStateFlashBase_qp", "PorousFlowMassFraction_qp");
  _deps.insertDependency("PorousFlowFluidStateFlashBase_qp", "PorousFlowDensity_qp");
  _deps.insertDependency("PorousFlowFluidStateFlashBase_qp", "PorousFlowViscosity_qp");
  _deps.insertDependency("PorousFlowVariableBase_nodal", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowVariableBase_qp", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowFluidStateWaterNCG_nodal", "PorousFlowFluidFlashBase_nodal");
  _deps.insertDependency("PorousFlowFluidStateWaterNCG_qp", "PorousFlowFluidFlashBase_qp");
  _deps.insertDependency("PorousFlowInternalEnergyIdeal_nodal",
                         "PorousFlowFluidPropertiesBase_nodal");
  _deps.insertDependency("PorousFlowInternalEnergyIdeal_qp", "PorousFlowFluidPropertiesBase_qp");
  _deps.insertDependency("PorousFlowMassFraction_nodal", "PorousFlowMaterialVectorBase_nodal");
  _deps.insertDependency("PorousFlowMassFraction_qp", "PorousFlowMaterialVectorBase_qp");
  _deps.insertDependency("PorousFlowMatrixInternalEnergy_nodal",
                         "PorousFlowMaterialVectorBase_nodal");
  _deps.insertDependency("PorousFlowMatrixInternalEnergy_nodal", "PorousFlowTemperature_nodal");
  _deps.insertDependency("PorousFlowMatrixInternalEnergy_qp", "PorousFlowMaterialVectorBase_qp");
  _deps.insertDependency("PorousFlowMatrixInternalEnergy_qp", "PorousFlowTemperature_qp");
  _deps.insertDependency("PorousFlowPermeability_nodal", "PorousFlowMaterialVectorBase_nodal");
  _deps.insertDependency("PorousFlowPermeability_nodal", "PorousFlowPorosity_nodal");
  _deps.insertDependency("PorousFlowPermeability_qp", "PorousFlowMaterialVectorBase_qp");
  _deps.insertDependency("PorousFlowPermeability_qp", "PorousFlowPorosity_qp");
  _deps.insertDependency("PorousFlowPorosity_nodal", "PorousFlowMaterialVectorBase_nodal");
  //_deps.insertDependency("PorousFlowPorosity_nodal", "PorousFlowVolumetricStrain_qp");
  _deps.insertDependency("PorousFlowPorosity_nodal", "PorousFlowEffectiveFluidPressure_nodal");
  _deps.insertDependency("PorousFlowPorosity_nodal", "PorousFlowTemperature_nodal");
  _deps.insertDependency("PorousFlowPorosity_qp", "PorousFlowMaterialVectorBase_qp");
  //_deps.insertDependency("PorousFlowPorosity_qp", "PorousFlowVolumetricStrain_qp");
  _deps.insertDependency("PorousFlowPorosity_qp", "PorousFlowEffectiveFluidPressure_qp");
  _deps.insertDependency("PorousFlowPorosity_qp", "PorousFlowTemperature_qp");
  _deps.insertDependency("PorousFlowRelativePermeability_nodal", "PorousFlowMaterialBase_nodal");
  _deps.insertDependency("PorousFlowRelativePermeability_nodal", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowRelativePermeability_qp", "PorousFlowMaterialBase_qp");
  _deps.insertDependency("PorousFlowRelativePermeability_qp", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowSingleComponentFluid_nodal",
                         "PorousFlowFluidPropertiesBase_nodal");
  _deps.insertDependency("PorousFlowSingleComponentFluid_qp", "PorousFlowFluidPropertiesBase_qp");
  _deps.insertDependency("PorousFlowThermalConductivityIdeal_nodal",
                         "PorousFlowMaterialVectorBase_nodal");
  _deps.insertDependency("PorousFlowThermalConductivityIdeal_nodal", "PorousFlowPS_nodal");
  _deps.insertDependency("PorousFlowThermalConductivityIdeal_qp",
                         "PorousFlowMaterialVectorBase_qp");
  _deps.insertDependency("PorousFlowThermalConductivityIdeal_qp", "PorousFlowPS_qp");
  _deps.insertDependency("PorousFlowViscosity_nodal", "porousFlowFluidPropertiesBase_nodal");
  _deps.insertDependency("PorousFlowViscosity_qp", "porousFlowFluidPropertiesBase_qp");
  _deps.insertDependency("PorousFlowVolumetricStrain_nodal", "PorousFlowMaterialVectorBase_nodal");
  _deps.insertDependency("PorousFlowVolumetricStrain_qp", "PorousFlowMaterialVectorBase_qp");
}
