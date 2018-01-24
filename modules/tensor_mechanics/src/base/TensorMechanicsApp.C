//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "CommonTensorMechanicsAction.h"
#include "TensorMechanicsAction.h"
#include "LegacyTensorMechanicsAction.h"
#include "DynamicTensorMechanicsAction.h"
#include "PoroMechanicsAction.h"
#include "PressureAction.h"
#include "GeneralizedPlaneStrainAction.h"

#include "StressDivergenceTensors.h"
#include "StressDivergenceTensorsTruss.h"
#include "CosseratStressDivergenceTensors.h"
#include "StressDivergenceRZTensors.h"
#include "StressDivergenceRSphericalTensors.h"
#include "MomentBalancing.h"
#include "PoroMechanicsCoupling.h"
#include "InertialForce.h"
#include "InertialTorque.h"
#include "Gravity.h"
#include "DynamicStressDivergenceTensors.h"
#include "OutOfPlanePressure.h"
#include "GeneralizedPlaneStrain.h"
#include "GeneralizedPlaneStrainOffDiag.h"
#include "WeakPlaneStress.h"
#include "PlasticHeatEnergy.h"
#include "PhaseFieldFractureMechanicsOffDiag.h"

#include "LinearElasticTruss.h"
#include "FiniteStrainPlasticMaterial.h"
#include "FiniteStrainCrystalPlasticity.h"
#include "FiniteStrainCPSlipRateRes.h"
#include "FiniteStrainUObasedCP.h"
#include "CappedMohrCoulombStressUpdate.h"
#include "CappedMohrCoulombCosseratStressUpdate.h"
#include "CappedWeakPlaneStressUpdate.h"
#include "CappedWeakInclinedPlaneStressUpdate.h"
#include "CappedWeakPlaneCosseratStressUpdate.h"
#include "CappedDruckerPragerStressUpdate.h"
#include "CappedDruckerPragerCosseratStressUpdate.h"
#include "ComputeMultiPlasticityStress.h"
#include "ComputeCosseratLinearElasticStress.h"
#include "ComputeCosseratSmallStrain.h"
#include "ComputeCosseratIncrementalSmallStrain.h"
#include "ComputeCosseratElasticityTensor.h"
#include "ComputeLayeredCosseratElasticityTensor.h"
#include "TwoPhaseStressMaterial.h"
#include "MultiPhaseStressMaterial.h"
#include "CompositeEigenstrain.h"
#include "CompositeElasticityTensor.h"
#include "ComputeElasticityTensor.h"
#include "ComputeElasticityTensorCP.h"
#include "ComputeIsotropicElasticityTensor.h"
#include "ComputeVariableIsotropicElasticityTensor.h"
#include "ComputeSmallStrain.h"
#include "ComputePlaneSmallStrain.h"
#include "ComputePlaneIncrementalStrain.h"
#include "ComputePlaneFiniteStrain.h"
#include "ComputeAxisymmetricRZSmallStrain.h"
#include "ComputeRSphericalSmallStrain.h"
#include "ComputeIncrementalSmallStrain.h"
#include "ComputeAxisymmetricRZIncrementalStrain.h"
#include "ComputeRSphericalIncrementalStrain.h"
#include "ComputeFiniteStrain.h"
#include "ComputeAxisymmetricRZFiniteStrain.h"
#include "ComputeRSphericalFiniteStrain.h"
#include "ComputeLinearElasticStress.h"
#include "ComputeIsotropicLinearElasticPFFractureStress.h"
#include "ComputeLinearElasticPFFractureStress.h"
#include "ComputeFiniteStrainElasticStress.h"
#include "ComputeEigenstrain.h"
#include "ComputeExtraStressConstant.h"
#include "ComputeVariableBaseEigenStrain.h"
#include "ComputeVariableEigenstrain.h"
#include "ComputeThermalExpansionEigenstrain.h"
#include "ComputeMeanThermalExpansionFunctionEigenstrain.h"
#include "ComputeInstantaneousThermalExpansionFunctionEigenstrain.h"
#include "ComputeReducedOrderEigenstrain.h"
#include "ComputeVolumetricEigenstrain.h"
#include "ComputeConcentrationDependentElasticityTensor.h"
#include "FiniteStrainHyperElasticViscoPlastic.h"
#include "LinearIsoElasticPFDamage.h"
#include "HyperElasticPhaseFieldIsoDamage.h"
#include "ComputeVolumetricDeformGrad.h"
#include "ComputeDeformGradBasedStress.h"
#include "VolumeDeformGradCorrectedStress.h"
#include "ComputeMultipleInelasticStress.h"
#include "ComputeMultipleInelasticCosseratStress.h"
#include "IsotropicPlasticityStressUpdate.h"
#include "IsotropicPowerLawHardeningStressUpdate.h"
#include "PowerLawCreepStressUpdate.h"
#include "HyperbolicViscoplasticityStressUpdate.h"
#include "TemperatureDependentHardeningStressUpdate.h"
#include "StressBasedChemicalPotential.h"
#include "FluxBasedStrainIncrement.h"
#include "GBRelaxationStrainIncrement.h"
#include "SumTensorIncrements.h"
#include "ComputeStrainIncrementBasedStress.h"
#include "ComputeSmearedCrackingStress.h"
#include "InclusionProperties.h"
#include "ComputeAxisymmetric1DSmallStrain.h"
#include "ComputeAxisymmetric1DIncrementalStrain.h"
#include "ComputeAxisymmetric1DFiniteStrain.h"
#include "ComputePlasticHeatEnergy.h"
#include "ComputeInterfaceStress.h"
#include "TensileStressUpdate.h"
#include "GeneralizedMaxwellModel.h"
#include "GeneralizedKelvinVoigtModel.h"
#include "LinearViscoelasticStressUpdate.h"
#include "ComputeLinearViscoelasticStress.h"
#include "ComputeEigenstrainFromInitialStress.h"

#include "TensorMechanicsPlasticSimpleTester.h"
#include "TensorMechanicsPlasticTensile.h"
#include "TensorMechanicsPlasticTensileMulti.h"
#include "TensorMechanicsPlasticMohrCoulomb.h"
#include "TensorMechanicsPlasticMohrCoulombMulti.h"
#include "TensorMechanicsPlasticWeakPlaneTensile.h"
#include "TensorMechanicsPlasticWeakPlaneTensileN.h"
#include "TensorMechanicsPlasticWeakPlaneShear.h"
#include "TensorMechanicsPlasticJ2.h"
#include "TensorMechanicsPlasticIsotropicSD.h"
#include "TensorMechanicsPlasticOrthotropic.h"
#include "TensorMechanicsPlasticMeanCap.h"
#include "TensorMechanicsPlasticMeanCapTC.h"
#include "TensorMechanicsPlasticDruckerPrager.h"
#include "TensorMechanicsPlasticDruckerPragerHyperbolic.h"
#include "TensorMechanicsHardeningConstant.h"
#include "TensorMechanicsHardeningGaussian.h"
#include "TensorMechanicsHardeningExponential.h"
#include "TensorMechanicsHardeningPowerRule.h"
#include "TensorMechanicsHardeningCutExponential.h"
#include "TensorMechanicsHardeningCubic.h"
#include "ElementPropertyReadFile.h"
#include "EulerAngleFileReader.h"
#include "HEVPLinearHardening.h"
#include "HEVPRambergOsgoodHardening.h"
#include "HEVPEqvPlasticStrain.h"
#include "HEVPEqvPlasticStrainRate.h"
#include "HEVPFlowRatePowerLawJ2.h"
#include "LinearViscoelasticityManager.h"

#include "CylindricalRankTwoAux.h"
#include "RankTwoAux.h"
#include "RankFourAux.h"
#include "ElasticEnergyAux.h"
#include "AccumulateAux.h"
#include "CrystalPlasticityRotationOutAux.h"
#include "RankTwoScalarAux.h"
#include "NewmarkAccelAux.h"
#include "NewmarkVelAux.h"
#include "RadialDisplacementCylinderAux.h"
#include "RadialDisplacementSphereAux.h"

#include "CavityPressureAction.h"
#include "CavityPressurePostprocessor.h"
#include "CavityPressurePPAction.h"
#include "CavityPressureUserObject.h"
#include "CavityPressureUOAction.h"

#include "DashpotBC.h"
#include "PresetVelocity.h"
#include "Pressure.h"
#include "DisplacementAboutAxis.h"
#include "PresetDisplacement.h"
#include "PresetAcceleration.h"
#include "StickyBC.h"

#include "CrystalPlasticitySlipRateGSS.h"
#include "CrystalPlasticitySlipResistanceGSS.h"
#include "CrystalPlasticityStateVariable.h"
#include "CrystalPlasticityStateVarRateComponentGSS.h"

#include "Mass.h"
#include "TorqueReaction.h"
#include "MaterialTensorIntegral.h"
#include "MaterialTimeStepPostprocessor.h"

#include "LineMaterialRankTwoSampler.h"
#include "LineMaterialRankTwoScalarSampler.h"

#include "GeneralizedPlaneStrainUserObject.h"

#include "ElementJacobianDamper.h"

#include "JIntegral.h"
#include "CrackDataSampler.h"
#include "CrackFrontData.h"
#include "CrackFrontDefinition.h"
#include "DomainIntegralAction.h"
#include "DomainIntegralQFunction.h"
#include "DomainIntegralTopologicalQFunction.h"
#include "InteractionIntegralBenchmarkBC.h"
#include "MixedModeEquivalentK.h"
#include "EshelbyTensor.h"
#include "InteractionIntegral.h"
#include "ThermalFractureIntegral.h"

template <>
InputParameters
validParams<TensorMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

TensorMechanicsApp::TensorMechanicsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  TensorMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  TensorMechanicsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  TensorMechanicsApp::registerExecFlags(_factory);
}

TensorMechanicsApp::~TensorMechanicsApp() {}

// External entry point for dynamic application loading
extern "C" void
TensorMechanicsApp_registerApps()
{
  TensorMechanicsApp::registerApps();
}
void
TensorMechanicsApp::registerApps()
{
  registerApp(TensorMechanicsApp);
}

// External entry point for dynamic object registration
extern "C" void
TensorMechanicsApp__registerObjects(Factory & factory)
{
  TensorMechanicsApp::registerObjects(factory);
}
void
TensorMechanicsApp::registerObjects(Factory & factory)
{
  registerKernel(StressDivergenceTensors);
  registerKernel(StressDivergenceTensorsTruss);
  registerKernel(CosseratStressDivergenceTensors);
  registerKernel(StressDivergenceRZTensors);
  registerKernel(StressDivergenceRSphericalTensors);
  registerKernel(MomentBalancing);
  registerKernel(PoroMechanicsCoupling);
  registerKernel(InertialForce);
  registerKernel(InertialTorque);
  registerKernel(Gravity);
  registerKernel(DynamicStressDivergenceTensors);
  registerKernel(OutOfPlanePressure);
  registerKernel(GeneralizedPlaneStrain);
  registerKernel(GeneralizedPlaneStrainOffDiag);
  registerKernel(WeakPlaneStress);
  registerKernel(PlasticHeatEnergy);
  registerKernel(PhaseFieldFractureMechanicsOffDiag);

  registerMaterial(LinearElasticTruss);
  registerMaterial(FiniteStrainPlasticMaterial);
  registerMaterial(FiniteStrainCrystalPlasticity);
  registerMaterial(FiniteStrainCPSlipRateRes);
  registerMaterial(FiniteStrainUObasedCP);
  registerMaterial(CappedMohrCoulombStressUpdate);
  registerMaterial(CappedMohrCoulombCosseratStressUpdate);
  registerMaterial(CappedWeakPlaneStressUpdate);
  registerMaterial(CappedWeakInclinedPlaneStressUpdate);
  registerMaterial(CappedWeakPlaneCosseratStressUpdate);
  registerMaterial(CappedDruckerPragerStressUpdate);
  registerMaterial(CappedDruckerPragerCosseratStressUpdate);
  registerMaterial(ComputeMultiPlasticityStress);
  registerMaterial(ComputeCosseratLinearElasticStress);
  registerMaterial(ComputeCosseratSmallStrain);
  registerMaterial(ComputeCosseratIncrementalSmallStrain);
  registerMaterial(ComputeCosseratElasticityTensor);
  registerMaterial(ComputeLayeredCosseratElasticityTensor);
  registerMaterial(TwoPhaseStressMaterial);
  registerMaterial(MultiPhaseStressMaterial);
  registerMaterial(CompositeEigenstrain);
  registerMaterial(CompositeElasticityTensor);
  registerMaterial(ComputeElasticityTensor);
  registerMaterial(ComputeElasticityTensorCP);
  registerMaterial(ComputeIsotropicElasticityTensor);
  registerMaterial(ComputeVariableIsotropicElasticityTensor);
  registerMaterial(ComputeSmallStrain);
  registerMaterial(ComputePlaneSmallStrain);
  registerMaterial(ComputePlaneIncrementalStrain);
  registerMaterial(ComputePlaneFiniteStrain);
  registerMaterial(ComputeAxisymmetricRZSmallStrain);
  registerMaterial(ComputeRSphericalSmallStrain);
  registerMaterial(ComputeIncrementalSmallStrain);
  registerMaterial(ComputeAxisymmetricRZIncrementalStrain);
  registerMaterial(ComputeRSphericalIncrementalStrain);
  registerMaterial(ComputeFiniteStrain);
  registerMaterial(ComputeAxisymmetricRZFiniteStrain);
  registerMaterial(ComputeRSphericalFiniteStrain);
  registerMaterial(ComputeLinearElasticStress);
  registerMaterial(ComputeIsotropicLinearElasticPFFractureStress);
  registerMaterial(ComputeLinearElasticPFFractureStress);
  registerMaterial(ComputeFiniteStrainElasticStress);
  registerMaterial(ComputeEigenstrain);
  registerMaterial(ComputeExtraStressConstant);
  registerMaterial(ComputeVariableBaseEigenStrain);
  registerMaterial(ComputeVariableEigenstrain);
  registerMaterial(ComputeThermalExpansionEigenstrain);
  registerMaterial(ComputeMeanThermalExpansionFunctionEigenstrain);
  registerMaterial(ComputeInstantaneousThermalExpansionFunctionEigenstrain);
  registerMaterial(ComputeReducedOrderEigenstrain);
  registerMaterial(ComputeVolumetricEigenstrain);
  registerMaterial(ComputeConcentrationDependentElasticityTensor);
  registerMaterial(FiniteStrainHyperElasticViscoPlastic);
  registerMaterial(LinearIsoElasticPFDamage);
  registerMaterial(HyperElasticPhaseFieldIsoDamage);
  registerMaterial(ComputeVolumetricDeformGrad);
  registerMaterial(ComputeDeformGradBasedStress);
  registerMaterial(VolumeDeformGradCorrectedStress);
  registerMaterial(ComputeMultipleInelasticStress);
  registerMaterial(ComputeMultipleInelasticCosseratStress);
  registerMaterial(IsotropicPlasticityStressUpdate);
  registerMaterial(IsotropicPowerLawHardeningStressUpdate);
  registerMaterial(PowerLawCreepStressUpdate);
  registerMaterial(HyperbolicViscoplasticityStressUpdate);
  registerMaterial(TemperatureDependentHardeningStressUpdate);
  registerMaterial(StressBasedChemicalPotential);
  registerMaterial(FluxBasedStrainIncrement);
  registerMaterial(GBRelaxationStrainIncrement);
  registerMaterial(SumTensorIncrements);
  registerMaterial(ComputeStrainIncrementBasedStress);
  registerMaterial(ComputeSmearedCrackingStress);
  registerMaterial(InclusionProperties);
  registerMaterial(ComputeAxisymmetric1DSmallStrain);
  registerMaterial(ComputeAxisymmetric1DIncrementalStrain);
  registerMaterial(ComputeAxisymmetric1DFiniteStrain);
  registerMaterial(ComputePlasticHeatEnergy);
  registerMaterial(ComputeInterfaceStress);
  registerMaterial(TensileStressUpdate);
  registerMaterial(EshelbyTensor);
  registerMaterial(GeneralizedMaxwellModel);
  registerMaterial(GeneralizedKelvinVoigtModel);
  registerMaterial(LinearViscoelasticStressUpdate);
  registerMaterial(ComputeLinearViscoelasticStress);
  registerMaterial(ThermalFractureIntegral);
  registerMaterial(ComputeEigenstrainFromInitialStress);

  registerUserObject(TensorMechanicsPlasticSimpleTester);
  registerUserObject(TensorMechanicsPlasticTensile);
  registerUserObject(TensorMechanicsPlasticTensileMulti);
  registerUserObject(TensorMechanicsPlasticMohrCoulomb);
  registerUserObject(TensorMechanicsPlasticMohrCoulombMulti);
  registerUserObject(TensorMechanicsPlasticWeakPlaneTensile);
  registerUserObject(TensorMechanicsPlasticWeakPlaneTensileN);
  registerUserObject(TensorMechanicsPlasticWeakPlaneShear);
  registerUserObject(TensorMechanicsPlasticJ2);
  registerUserObject(TensorMechanicsPlasticIsotropicSD);
  registerUserObject(TensorMechanicsPlasticOrthotropic);
  registerUserObject(TensorMechanicsPlasticMeanCap);
  registerUserObject(TensorMechanicsPlasticMeanCapTC);
  registerUserObject(TensorMechanicsPlasticDruckerPrager);
  registerUserObject(TensorMechanicsPlasticDruckerPragerHyperbolic);
  registerUserObject(TensorMechanicsHardeningConstant);
  registerUserObject(TensorMechanicsHardeningGaussian);
  registerUserObject(TensorMechanicsHardeningExponential);
  registerUserObject(TensorMechanicsHardeningPowerRule);
  registerUserObject(TensorMechanicsHardeningCutExponential);
  registerUserObject(TensorMechanicsHardeningCubic);
  registerUserObject(ElementPropertyReadFile);
  registerUserObject(EulerAngleFileReader);
  registerUserObject(HEVPLinearHardening);
  registerUserObject(HEVPRambergOsgoodHardening);
  registerUserObject(HEVPEqvPlasticStrain);
  registerUserObject(HEVPEqvPlasticStrainRate);
  registerUserObject(HEVPFlowRatePowerLawJ2);
  registerUserObject(CavityPressureUserObject);
  registerUserObject(CrystalPlasticitySlipRateGSS);
  registerUserObject(CrystalPlasticitySlipResistanceGSS);
  registerUserObject(CrystalPlasticityStateVariable);
  registerUserObject(CrystalPlasticityStateVarRateComponentGSS);
  registerUserObject(GeneralizedPlaneStrainUserObject);
  registerUserObject(CrackFrontDefinition);
  registerUserObject(LinearViscoelasticityManager);

  registerAux(CylindricalRankTwoAux);
  registerAux(RankTwoAux);
  registerAux(RankFourAux);
  registerAux(ElasticEnergyAux);
  registerAux(AccumulateAux);
  registerAux(CrystalPlasticityRotationOutAux);
  registerAux(RankTwoScalarAux);
  registerAux(NewmarkAccelAux);
  registerAux(NewmarkVelAux);
  registerAux(RadialDisplacementCylinderAux);
  registerAux(RadialDisplacementSphereAux);
  registerAux(DomainIntegralQFunction);
  registerAux(DomainIntegralTopologicalQFunction);

  registerBoundaryCondition(DashpotBC);
  registerBoundaryCondition(PresetVelocity);
  registerBoundaryCondition(Pressure);
  registerBoundaryCondition(DisplacementAboutAxis);
  registerBoundaryCondition(PresetDisplacement);
  registerBoundaryCondition(PresetAcceleration);
  registerBoundaryCondition(InteractionIntegralBenchmarkBC);
  registerBoundaryCondition(StickyBC);

  registerPostprocessor(CavityPressurePostprocessor);
  registerPostprocessor(Mass);
  registerPostprocessor(TorqueReaction);
  registerPostprocessor(MaterialTensorIntegral);
  registerPostprocessor(MaterialTimeStepPostprocessor);
  registerPostprocessor(JIntegral);
  registerPostprocessor(InteractionIntegral);
  registerPostprocessor(CrackFrontData);
  registerPostprocessor(MixedModeEquivalentK);

  registerVectorPostprocessor(LineMaterialRankTwoSampler);
  registerVectorPostprocessor(LineMaterialRankTwoScalarSampler);
  registerVectorPostprocessor(CrackDataSampler);

  registerDamper(ElementJacobianDamper);
}

// External entry point for dynamic syntax association
extern "C" void
TensorMechanicsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
}
void
TensorMechanicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerSyntax("EmptyAction", "BCs/CavityPressure");
  registerSyntax("CavityPressureAction", "BCs/CavityPressure/*");
  registerSyntax("CavityPressurePPAction", "BCs/CavityPressure/*");
  registerSyntax("CavityPressureUOAction", "BCs/CavityPressure/*");

  registerSyntax("LegacyTensorMechanicsAction", "Kernels/TensorMechanics");
  registerSyntax("DynamicTensorMechanicsAction", "Kernels/DynamicTensorMechanics");
  registerSyntax("PoroMechanicsAction", "Kernels/PoroMechanics");

  registerSyntax("EmptyAction", "BCs/Pressure");
  registerSyntax("PressureAction", "BCs/Pressure/*");

  registerSyntax("GeneralizedPlaneStrainAction",
                 "Modules/TensorMechanics/GeneralizedPlaneStrain/*");
  registerSyntax("CommonTensorMechanicsAction", "Modules/TensorMechanics/Master");
  registerSyntax("TensorMechanicsAction", "Modules/TensorMechanics/Master/*");

  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_user_object");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_aux_variable");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_aux_kernel");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_postprocessor");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_vector_postprocessor");
  registerSyntaxTask("DomainIntegralAction", "DomainIntegral", "add_material");

  registerTask("validate_coordinate_systems", /*is_required=*/false);
  addTaskDependency("validate_coordinate_systems", "create_problem");

  registerAction(CavityPressureAction, "add_bc");
  registerAction(CavityPressurePPAction, "add_postprocessor");
  registerAction(CavityPressureUOAction, "add_user_object");

  registerAction(LegacyTensorMechanicsAction, "setup_mesh_complete");
  registerAction(LegacyTensorMechanicsAction, "validate_coordinate_systems");
  registerAction(LegacyTensorMechanicsAction, "add_kernel");

  registerAction(CommonTensorMechanicsAction, "meta_action");

  registerAction(TensorMechanicsAction, "meta_action");
  registerAction(TensorMechanicsAction, "setup_mesh_complete");
  registerAction(TensorMechanicsAction, "validate_coordinate_systems");
  registerAction(TensorMechanicsAction, "add_variable");
  registerAction(TensorMechanicsAction, "add_aux_variable");
  registerAction(TensorMechanicsAction, "add_kernel");
  registerAction(TensorMechanicsAction, "add_aux_kernel");
  registerAction(TensorMechanicsAction, "add_material");

  registerAction(DynamicTensorMechanicsAction, "setup_mesh_complete");
  registerAction(DynamicTensorMechanicsAction, "validate_coordinate_systems");
  registerAction(DynamicTensorMechanicsAction, "add_kernel");

  registerAction(PoroMechanicsAction, "setup_mesh_complete");
  registerAction(PoroMechanicsAction, "validate_coordinate_systems");
  registerAction(PoroMechanicsAction, "add_kernel");

  registerAction(PressureAction, "add_bc");

  registerAction(GeneralizedPlaneStrainAction, "add_scalar_kernel");
  registerAction(GeneralizedPlaneStrainAction, "add_kernel");
  registerAction(GeneralizedPlaneStrainAction, "add_user_object");

  registerAction(DomainIntegralAction, "add_user_object");
  registerAction(DomainIntegralAction, "add_aux_variable");
  registerAction(DomainIntegralAction, "add_aux_kernel");
  registerAction(DomainIntegralAction, "add_postprocessor");
  registerAction(DomainIntegralAction, "add_material");
}

// External entry point for dynamic execute flag registration
extern "C" void
TensorMechanicsApp__registerExecFlags(Factory & factory)
{
  TensorMechanicsApp::registerExecFlags(factory);
}
void
TensorMechanicsApp::registerExecFlags(Factory & /*factory*/)
{
}
