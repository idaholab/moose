/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
#include "ComputeCappedWeakPlaneStress.h"
#include "ComputeCappedWeakInclinedPlaneStress.h"
#include "ComputeCappedWeakPlaneCosseratStress.h"
#include "ComputeCappedDruckerPragerStress.h"
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
#include "ComputeLinearElasticPFFractureStress.h"
#include "ComputeFiniteStrainElasticStress.h"
#include "ComputeVariableElasticConstantStress.h"
#include "ComputeEigenstrain.h"
#include "ComputeExtraStressConstant.h"
#include "ComputeVariableBaseEigenStrain.h"
#include "ComputeVariableEigenstrain.h"
#include "ComputeThermalExpansionEigenstrain.h"
#include "ComputeMeanThermalExpansionFunctionEigenstrain.h"
#include "ComputeInstantaneousThermalExpansionFunctionEigenstrain.h"
#include "ComputeVolumetricEigenstrain.h"
#include "ComputeConcentrationDependentElasticityTensor.h"
#include "FiniteStrainHyperElasticViscoPlastic.h"
#include "LinearIsoElasticPFDamage.h"
#include "HyperElasticPhaseFieldIsoDamage.h"
#include "ComputeVolumetricDeformGrad.h"
#include "ComputeDeformGradBasedStress.h"
#include "VolumeDeformGradCorrectedStress.h"
#include "ComputeReturnMappingStress.h"
#include "RadialReturnStressUpdate.h"
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
#include "ComputeElasticSmearedCrackingStress.h"
#include "InclusionProperties.h"
#include "ComputeAxisymmetric1DSmallStrain.h"
#include "ComputeAxisymmetric1DIncrementalStrain.h"
#include "ComputeAxisymmetric1DFiniteStrain.h"
#include "ComputePlasticHeatEnergy.h"

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

#include "CylindricalRankTwoAux.h"
#include "RankTwoAux.h"
#include "RankFourAux.h"
#include "ElasticEnergyAux.h"
#include "AccumulateAux.h"
#include "CrystalPlasticityRotationOutAux.h"
#include "RankTwoScalarAux.h"
#include "StressDivergencePFFracTensors.h"
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

#include "CrystalPlasticitySlipRateGSS.h"
#include "CrystalPlasticitySlipResistanceGSS.h"
#include "CrystalPlasticityStateVariable.h"
#include "CrystalPlasticityStateVarRateComponentGSS.h"

#include "Mass.h"
#include "TorqueReaction.h"
#include "MaterialTensorIntegral.h"

#include "LineMaterialRankTwoSampler.h"
#include "LineMaterialRankTwoScalarSampler.h"

#include "GeneralizedPlaneStrainUserObject.h"

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

  Moose::registerExecFlags();
  TensorMechanicsApp::registerExecFlags();
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
  registerDeprecatedObject(StressDivergencePFFracTensors, "06/01/2017 09:00");
  registerKernel(PoroMechanicsCoupling);
  registerKernel(InertialForce);
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
  registerMaterial(ComputeCappedWeakPlaneStress);
  registerMaterial(ComputeCappedWeakInclinedPlaneStress);
  registerMaterial(ComputeCappedWeakPlaneCosseratStress);
  registerMaterial(ComputeCappedDruckerPragerStress);
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
  registerMaterial(ComputeLinearElasticPFFractureStress);
  registerMaterial(ComputeFiniteStrainElasticStress);
  registerMaterial(ComputeVariableElasticConstantStress);
  registerMaterial(ComputeEigenstrain);
  registerMaterial(ComputeExtraStressConstant);
  registerMaterial(ComputeVariableBaseEigenStrain);
  registerMaterial(ComputeVariableEigenstrain);
  registerDeprecatedObjectName(
      ComputeThermalExpansionEigenstrain, "ComputeThermalExpansionEigenStrain", "12/19/2016 00:00");
  registerMaterial(ComputeThermalExpansionEigenstrain);
  registerMaterial(ComputeMeanThermalExpansionFunctionEigenstrain);
  registerMaterial(ComputeInstantaneousThermalExpansionFunctionEigenstrain);
  registerMaterial(ComputeVolumetricEigenstrain);
  registerMaterial(ComputeConcentrationDependentElasticityTensor);
  registerMaterial(FiniteStrainHyperElasticViscoPlastic);
  registerMaterial(LinearIsoElasticPFDamage);
  registerMaterial(HyperElasticPhaseFieldIsoDamage);
  registerMaterial(ComputeVolumetricDeformGrad);
  registerMaterial(ComputeDeformGradBasedStress);
  registerMaterial(VolumeDeformGradCorrectedStress);
  registerMaterial(ComputeReturnMappingStress);
  registerMaterial(RadialReturnStressUpdate);
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
  registerMaterial(ComputeElasticSmearedCrackingStress);
  registerMaterial(InclusionProperties);
  registerMaterial(ComputeAxisymmetric1DSmallStrain);
  registerMaterial(ComputeAxisymmetric1DIncrementalStrain);
  registerMaterial(ComputeAxisymmetric1DFiniteStrain);
  registerMaterial(ComputePlasticHeatEnergy);

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

  registerBoundaryCondition(DashpotBC);
  registerBoundaryCondition(PresetVelocity);
  registerBoundaryCondition(Pressure);
  registerBoundaryCondition(DisplacementAboutAxis);
  registerBoundaryCondition(PresetDisplacement);
  registerBoundaryCondition(PresetAcceleration);

  registerPostprocessor(CavityPressurePostprocessor);
  registerPostprocessor(Mass);
  registerPostprocessor(TorqueReaction);
  registerPostprocessor(MaterialTensorIntegral);

  registerVectorPostprocessor(LineMaterialRankTwoSampler);
  registerVectorPostprocessor(LineMaterialRankTwoScalarSampler);
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
}

void
TensorMechanicsApp::registerExecFlags()
{
}
