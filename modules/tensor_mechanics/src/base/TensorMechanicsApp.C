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

#include "TensorMechanicsAction.h"
#include "DynamicTensorMechanicsAction.h"
#include "TensorMechanicsAxisymmetricRZAction.h"
#include "TensorMechanicsRSphericalAction.h"
#include "PoroMechanicsAction.h"
#include "PressureAction.h"

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

#include "LinearElasticMaterial.h"
#include "LinearElasticTruss.h"
#include "FiniteStrainPlasticMaterial.h"
#include "FiniteStrainRatePlasticMaterial.h"
#include "FiniteStrainCrystalPlasticity.h"
#include "FiniteStrainCPSlipRateRes.h"
#include "FiniteStrainUObasedCP.h"
#include "ComputeMultiPlasticityStress.h"
#include "ComputeCosseratLinearElasticStress.h"
#include "ComputeCosseratSmallStrain.h"
#include "ComputeCosseratElasticityTensor.h"
#include "ComputeLayeredCosseratElasticityTensor.h"
#include "TwoPhaseStressMaterial.h"
#include "MultiPhaseStressMaterial.h"
#include "SimpleEigenStrainMaterial.h"
#include "CompositeEigenstrain.h"
#include "CompositeElasticityTensor.h"
#include "ComputeElasticityTensor.h"
#include "ComputeElasticityTensorCP.h"
#include "ComputeIsotropicElasticityTensor.h"
#include "ComputeSmallStrain.h"
#include "ComputePlaneSmallStrain.h"
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
#include "ComputeFiniteStrainElasticStress.h"
#include "ComputeEigenstrain.h"
#include "ComputeVariableBaseEigenStrain.h"
#include "ComputeVariableEigenstrain.h"
#include "ComputeThermalExpansionEigenStrain.h"
#include "ComputeConcentrationDependentElasticityTensor.h"
#include "FiniteStrainHyperElasticViscoPlastic.h"
#include "LinearIsoElasticPFDamage.h"
#include "HyperElasticPhaseFieldIsoDamage.h"
#include "ComputeVolumetricDeformGrad.h"
#include "ComputeDeformGradBasedStress.h"
#include "VolumeDeformGradCorrectedStress.h"
#include "ComputeReturnMappingStress.h"
#include "RecomputeRadialReturn.h"
#include "RecomputeRadialReturnIsotropicPlasticity.h"
#include "RecomputeRadialReturnPowerLawCreep.h"
#include "RecomputeRadialReturnTempDepHardening.h"
#include "StressBasedChemicalPotential.h"
#include "FluxBasedStrainIncrement.h"
#include "GBRelaxationStrainIncrement.h"
#include "SumTensorIncrements.h"
#include "ComputeStrainIncrementBasedStress.h"
#include "ComputeElasticSmearedCrackingStress.h"

#include "TensorMechanicsPlasticSimpleTester.h"
#include "TensorMechanicsPlasticTensile.h"
#include "TensorMechanicsPlasticTensileMulti.h"
#include "TensorMechanicsPlasticMohrCoulomb.h"
#include "TensorMechanicsPlasticMohrCoulombMulti.h"
#include "TensorMechanicsPlasticWeakPlaneTensile.h"
#include "TensorMechanicsPlasticWeakPlaneTensileN.h"
#include "TensorMechanicsPlasticWeakPlaneShear.h"
#include "TensorMechanicsPlasticJ2.h"
#include "TensorMechanicsPlasticMeanCap.h"
#include "TensorMechanicsPlasticDruckerPragerHyperbolic.h"
#include "TensorMechanicsHardeningConstant.h"
#include "TensorMechanicsHardeningGaussian.h"
#include "TensorMechanicsHardeningExponential.h"
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

#include "CavityPressureAction.h"
#include "CavityPressurePostprocessor.h"
#include "CavityPressurePPAction.h"
#include "CavityPressureUserObject.h"
#include "CavityPressureUOAction.h"

#include "DashpotBC.h"
#include "PresetVelocity.h"
#include "Pressure.h"
#include "DisplacementAboutAxis.h"

#include "CrystalPlasticitySlipRateGSS.h"
#include "CrystalPlasticitySlipResistanceGSS.h"
#include "CrystalPlasticityStateVariable.h"
#include "CrystalPlasticityStateVarRateComponentGSS.h"

#include "Mass.h"
#include "TorqueReaction.h"
#include "MaterialTensorIntegral.h"

#include "LineMaterialRankTwoSampler.h"
#include "LineMaterialRankTwoScalarSampler.h"

template<>
InputParameters validParams<TensorMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}

TensorMechanicsApp::TensorMechanicsApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  TensorMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  TensorMechanicsApp::associateSyntax(_syntax, _action_factory);
}

TensorMechanicsApp::~TensorMechanicsApp()
{
}

// External entry point for dynamic application loading
extern "C" void TensorMechanicsApp_registerApps() { TensorMechanicsApp::registerApps(); }
void
TensorMechanicsApp::registerApps()
{
  registerApp(TensorMechanicsApp);
}

// External entry point for dynamic object registration
extern "C" void TensorMechanicsApp__registerObjects(Factory & factory) { TensorMechanicsApp::registerObjects(factory); }
void
TensorMechanicsApp::registerObjects(Factory & factory)
{
  registerKernel(StressDivergenceTensors);
  registerKernel(StressDivergenceTensorsTruss);
  registerKernel(CosseratStressDivergenceTensors);
  registerKernel(StressDivergenceRZTensors);
  registerKernel(StressDivergenceRSphericalTensors);
  registerKernel(MomentBalancing);
  registerKernel(StressDivergencePFFracTensors);
  registerKernel(PoroMechanicsCoupling);
  registerKernel(InertialForce);
  registerKernel(Gravity);
  registerKernel(DynamicStressDivergenceTensors);
  registerKernel(OutOfPlanePressure);

  registerMaterial(LinearElasticMaterial);
  registerMaterial(LinearElasticTruss);
  registerMaterial(FiniteStrainPlasticMaterial);
  registerMaterial(FiniteStrainRatePlasticMaterial);
  registerMaterial(FiniteStrainCrystalPlasticity);
  registerMaterial(FiniteStrainCPSlipRateRes);
  registerMaterial(FiniteStrainUObasedCP);
  registerMaterial(ComputeMultiPlasticityStress);
  registerMaterial(ComputeCosseratLinearElasticStress);
  registerMaterial(ComputeCosseratSmallStrain);
  registerMaterial(ComputeCosseratElasticityTensor);
  registerMaterial(ComputeLayeredCosseratElasticityTensor);
  registerMaterial(TwoPhaseStressMaterial);
  registerMaterial(MultiPhaseStressMaterial);
  registerMaterial(SimpleEigenStrainMaterial);
  registerMaterial(CompositeEigenstrain);
  registerMaterial(CompositeElasticityTensor);
  registerMaterial(ComputeElasticityTensor);
  registerMaterial(ComputeElasticityTensorCP);
  registerMaterial(ComputeIsotropicElasticityTensor);
  registerMaterial(ComputeSmallStrain);
  registerMaterial(ComputePlaneSmallStrain);
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
  registerMaterial(ComputeFiniteStrainElasticStress);
  registerMaterial(ComputeEigenstrain);
  registerMaterial(ComputeVariableBaseEigenStrain);
  registerMaterial(ComputeVariableEigenstrain);
  registerMaterial(ComputeThermalExpansionEigenStrain);
  registerMaterial(ComputeConcentrationDependentElasticityTensor);
  registerMaterial(FiniteStrainHyperElasticViscoPlastic);
  registerMaterial(LinearIsoElasticPFDamage);
  registerMaterial(HyperElasticPhaseFieldIsoDamage);
  registerMaterial(ComputeVolumetricDeformGrad);
  registerMaterial(ComputeDeformGradBasedStress);
  registerMaterial(VolumeDeformGradCorrectedStress);
  registerMaterial(ComputeReturnMappingStress);
  registerMaterial(RecomputeRadialReturn);
  registerMaterial(RecomputeRadialReturnIsotropicPlasticity);
  registerMaterial(RecomputeRadialReturnPowerLawCreep);
  registerMaterial(RecomputeRadialReturnTempDepHardening);
  registerMaterial(StressBasedChemicalPotential);
  registerMaterial(FluxBasedStrainIncrement);
  registerMaterial(GBRelaxationStrainIncrement);
  registerMaterial(SumTensorIncrements);
  registerMaterial(ComputeStrainIncrementBasedStress);
  registerMaterial(ComputeElasticSmearedCrackingStress);

  registerUserObject(TensorMechanicsPlasticSimpleTester);
  registerUserObject(TensorMechanicsPlasticTensile);
  registerUserObject(TensorMechanicsPlasticTensileMulti);
  registerUserObject(TensorMechanicsPlasticMohrCoulomb);
  registerUserObject(TensorMechanicsPlasticMohrCoulombMulti);
  registerUserObject(TensorMechanicsPlasticWeakPlaneTensile);
  registerUserObject(TensorMechanicsPlasticWeakPlaneTensileN);
  registerUserObject(TensorMechanicsPlasticWeakPlaneShear);
  registerUserObject(TensorMechanicsPlasticJ2);
  registerUserObject(TensorMechanicsPlasticMeanCap);
  registerUserObject(TensorMechanicsPlasticDruckerPragerHyperbolic);
  registerUserObject(TensorMechanicsHardeningConstant);
  registerUserObject(TensorMechanicsHardeningGaussian);
  registerUserObject(TensorMechanicsHardeningExponential);
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

  registerAux(CylindricalRankTwoAux);
  registerAux(RankTwoAux);
  registerAux(RankFourAux);
  registerAux(ElasticEnergyAux);
  registerAux(AccumulateAux);
  registerAux(CrystalPlasticityRotationOutAux);
  registerAux(RankTwoScalarAux);
  registerAux(NewmarkAccelAux);
  registerAux(NewmarkVelAux);

  registerBoundaryCondition(DashpotBC);
  registerBoundaryCondition(PresetVelocity);
  registerBoundaryCondition(Pressure);
  registerBoundaryCondition(DisplacementAboutAxis);

  registerPostprocessor(CavityPressurePostprocessor);
  registerPostprocessor(Mass);
  registerPostprocessor(TorqueReaction);
  registerPostprocessor(MaterialTensorIntegral);

  registerVectorPostprocessor(LineMaterialRankTwoSampler);
  registerVectorPostprocessor(LineMaterialRankTwoScalarSampler);
}

// External entry point for dynamic syntax association
extern "C" void TensorMechanicsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { TensorMechanicsApp::associateSyntax(syntax, action_factory); }
void
TensorMechanicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  syntax.registerActionSyntax("EmptyAction", "BCs/CavityPressure");
  syntax.registerActionSyntax("CavityPressureAction", "BCs/CavityPressure/*");
  syntax.registerActionSyntax("CavityPressurePPAction", "BCs/CavityPressure/*");
  syntax.registerActionSyntax("CavityPressureUOAction", "BCs/CavityPressure/*");

  syntax.registerActionSyntax("TensorMechanicsAction", "Kernels/TensorMechanics");
  syntax.registerActionSyntax("DynamicTensorMechanicsAction", "Kernels/DynamicTensorMechanics");
  syntax.registerActionSyntax("PoroMechanicsAction", "Kernels/PoroMechanics");
  syntax.registerActionSyntax("TensorMechanicsAxisymmetricRZAction", "Kernels/StressDivergence2DAxisymmetricRZ");
  syntax.registerActionSyntax("TensorMechanicsRSphericalAction", "Kernels/StressDivergence1DRSpherical");

  syntax.registerActionSyntax("EmptyAction", "BCs/Pressure");
  syntax.registerActionSyntax("PressureAction", "BCs/Pressure/*");

  registerAction(CavityPressureAction, "add_bc");
  registerAction(CavityPressurePPAction, "add_postprocessor");
  registerAction(CavityPressureUOAction, "add_user_object");
  registerAction(TensorMechanicsAction, "add_kernel");
  registerAction(DynamicTensorMechanicsAction, "add_kernel");
  registerAction(PoroMechanicsAction, "add_kernel");
  registerAction(TensorMechanicsAxisymmetricRZAction, "add_kernel");
  registerAction(TensorMechanicsRSphericalAction, "add_kernel");
  registerAction(PressureAction, "add_bc");
}
