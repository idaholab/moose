/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PhaseFieldApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

/*
 * Kernels
 */
#include "ACGBPoly.h"
#include "ACGrGrElasticDrivingForce.h"
#include "ACGrGrMulti.h"
#include "ACGrGrPoly.h"
#include "ACInterface.h"
#include "ACMultiInterface.h"
#include "ACInterfaceKobayashi1.h"
#include "ACInterfaceKobayashi2.h"
#include "AllenCahnPFFracture.h"
#include "ACSEDGPoly.h"
#include "ACSwitching.h"
#include "AllenCahn.h"
#include "CahnHilliard.h"
#include "CahnHilliardAniso.h"
#include "CHBulkPFCTrad.h"
#include "CHCpldPFCTrad.h"
#include "CHInterface.h"
#include "CHInterfaceAniso.h"
#include "CHMath.h"
#include "CHPFCRFF.h"
#include "CHSplitChemicalPotential.h"
#include "CHSplitConcentration.h"
#include "CHSplitFlux.h"
#include "LaplacianSplit.h"
#include "CoefCoupledTimeDerivative.h"
#include "ConservedLangevinNoise.h"
#include "CoupledAllenCahn.h"
#include "CoupledSusceptibilityTimeDerivative.h"
#include "CoupledSwitchingTimeDerivative.h"
#include "GradientComponent.h"
#include "HHPFCRFF.h"
#include "KKSACBulkC.h"
#include "KKSACBulkF.h"
#include "KKSCHBulk.h"
#include "KKSMultiACBulkC.h"
#include "KKSMultiACBulkF.h"
#include "KKSMultiPhaseConcentration.h"
#include "KKSPhaseChemicalPotential.h"
#include "KKSPhaseConcentration.h"
#include "KKSSplitCHCRes.h"
#include "LangevinNoise.h"
#include "MaskedBodyForce.h"
#include "MatAnisoDiffusion.h"
#include "MatDiffusion.h"
#include "MatReaction.h"
#include "MultiGrainRigidBodyMotion.h"
#include "PFFracBulkRate.h"
#include "PFFracCoupledInterface.h"
#include "SimpleACInterface.h"
#include "SimpleCHInterface.h"
#include "SimpleCoupledACInterface.h"
#include "SimpleSplitCHWRes.h"
#include "SingleGrainRigidBodyMotion.h"
#include "SoretDiffusion.h"
#include "SplitCHMath.h"
#include "SplitCHParsed.h"
#include "SplitCHWRes.h"
#include "SplitCHWResAniso.h"
#include "SusceptibilityTimeDerivative.h"
#include "SwitchingFunctionConstraintEta.h"
#include "SwitchingFunctionConstraintLagrange.h"
#include "SwitchingFunctionPenalty.h"

// Remove this once the PFFracIntVar -> Reaction deprecation is expired:
#include "Reaction.h"

/*
 * Initial Conditions
 */
#include "BimodalInverseSuperellipsoidsIC.h"
#include "BimodalSuperellipsoidsIC.h"
#include "ClosePackIC.h"
#include "CrossIC.h"
#include "HexPolycrystalIC.h"
#include "LatticeSmoothCircleIC.h"
#include "MultiBoundingBoxIC.h"
#include "MultiSmoothCircleIC.h"
#include "MultiSmoothSuperellipsoidIC.h"
#include "PFCFreezingIC.h"
#include "PolycrystalRandomIC.h"
#include "PolycrystalReducedIC.h"
#include "PolycrystalVoronoiVoidIC.h"
#include "RampIC.h"
#include "ReconPhaseVarIC.h"
#include "ReconVarIC.h"
#include "RndBoundingBoxIC.h"
#include "RndSmoothCircleIC.h"
#include "SmoothCircleIC.h"
#include "SmoothSuperellipsoidIC.h"
#include "SpecifiedSmoothCircleIC.h"
#include "SpecifiedSmoothSuperellipsoidIC.h"
#include "ThumbIC.h"
#include "Tricrystal2CircleGrainsIC.h"
#include "TricrystalTripleJunctionIC.h"

/*
 * Boundary Conditions
 */
#include "CahnHilliardAnisoFluxBC.h"
#include "CahnHilliardFluxBC.h"

/*
 * InterfaceKernels
 */
#include "EqualGradientLagrangeInterface.h"
#include "EqualGradientLagrangeMultiplier.h"
#include "InterfaceDiffusionBoundaryTerm.h"
#include "InterfaceDiffusionFluxMatch.h"

/*
 * Materials
 */
#include "AsymmetricCrossTermBarrierFunctionMaterial.h"
#include "BarrierFunctionMaterial.h"
#include "CompositeMobilityTensor.h"
#include "ComputePolycrystalElasticityTensor.h"
#include "ConstantAnisotropicMobility.h"
#include "CrossTermBarrierFunctionMaterial.h"
#include "DeformedGrainMaterial.h"
#include "DerivativeMultiPhaseMaterial.h"
#include "DerivativeParsedMaterial.h"
#include "DerivativeSumMaterial.h"
#include "DerivativeTwoPhaseMaterial.h"
#include "DiscreteNucleation.h"
#include "ElasticEnergyMaterial.h"
#include "ExternalForceDensityMaterial.h"
#include "ForceDensityMaterial.h"
#include "GBAnisotropy.h"
#include "GBDependentAnisotropicTensor.h"
#include "GBDependentDiffusivity.h"
#include "GBEvolution.h"
#include "GBWidthAnisotropy.h"
#include "GrainAdvectionVelocity.h"
#include "InterfaceOrientationMaterial.h"
#include "KKSXeVacSolidMaterial.h"
#include "MathEBFreeEnergy.h"
#include "MathFreeEnergy.h"
#include "MultiBarrierFunctionMaterial.h"
#include "ParsedMaterial.h"
#include "PFCRFFMaterial.h"
#include "PFCTradMaterial.h"
#include "PFFracBulkRateMaterial.h"
#include "PFMobility.h"
#include "PFParamsPolyFreeEnergy.h"
#include "PhaseNormalTensor.h"
#include "PolynomialFreeEnergy.h"
#include "RegularSolutionFreeEnergy.h"
#include "StrainGradDispDerivatives.h"
#include "SwitchingFunction3PhaseMaterial.h"
#include "SwitchingFunctionMaterial.h"
#include "SwitchingFunctionMultiPhaseMaterial.h"
#include "ThirdPhaseSuppressionMaterial.h"
#include "TimeStepMaterial.h"
#include "VariableGradientMaterial.h"

/*
 * Postprocessors
 */
#include "FeatureFloodCount.h"
#include "GrainBoundaryArea.h"
#include "GrainTracker.h"
#include "GrainTrackerElasticity.h"
#include "FauxGrainTracker.h"
#include "PFCElementEnergyIntegral.h"

/*
 * AuxKernels
 */
#include "BndsCalcAux.h"
#include "CrossTermGradientFreeEnergy.h"
#include "EulerAngleVariables2RGBAux.h"
#include "FeatureFloodCountAux.h"
#include "GrainAdvectionAux.h"
#include "KKSGlobalFreeEnergy.h"
#include "KKSMultiFreeEnergy.h"
#include "PFCEnergyDensity.h"
#include "PFCRFFEnergyDensity.h"
#include "EBSDReaderAvgDataAux.h"
#include "EBSDReaderPointDataAux.h"
#include "TotalFreeEnergy.h"
#include "OutputEulerAngles.h"
#include "EulerAngleProvider2RGBAux.h"

/*
 * Functions
 */

/*
 * User Objects
 */
#include "ComputeExternalGrainForceAndTorque.h"
#include "ComputeGrainCenterUserObject.h"
#include "ComputeGrainForceAndTorque.h"
#include "ConservedMaskedNormalNoise.h"
#include "ConservedMaskedUniformNoise.h"
#include "ConservedNormalNoise.h"
#include "ConservedUniformNoise.h"
#include "ConstantGrainForceAndTorque.h"
#include "DiscreteNucleationInserter.h"
#include "DiscreteNucleationMap.h"
#include "EulerAngleUpdater.h"
#include "GrainForceAndTorqueSum.h"
#include "MaskedGrainForceAndTorque.h"
#include "RandomEulerAngleProvider.h"

#include "EBSDReader.h"
#include "SolutionRasterizer.h"

/*
 * Meshes
 */
#include "EBSDMesh.h"
#include "MortarPeriodicMesh.h"

/*
 * Actions
 */
#include "BicrystalBoundingBoxICAction.h"
#include "BicrystalCircleGrainICAction.h"
#include "CHPFCRFFSplitKernelAction.h"
#include "CHPFCRFFSplitVariablesAction.h"
#include "DisplacementGradientsAction.h"
#include "EulerAngle2RGBAction.h"
#include "HHPFCRFFSplitKernelAction.h"
#include "HHPFCRFFSplitVariablesAction.h"
#include "MaterialVectorAuxKernelAction.h"
#include "MaterialVectorGradAuxKernelAction.h"
#include "MatVecRealGradAuxKernelAction.h"
#include "MortarPeriodicAction.h"
#include "MultiAuxVariablesAction.h"
#include "PFCRFFKernelAction.h"
#include "PFCRFFVariablesAction.h"
#include "PolycrystalElasticDrivingForceAction.h"
#include "PolycrystalHexGrainICAction.h"
#include "PolycrystalKernelAction.h"
#include "PolycrystalRandomICAction.h"
#include "PolycrystalStoredEnergyAction.h"
#include "PolycrystalVariablesAction.h"
#include "PolycrystalVoronoiICAction.h"
#include "PolycrystalVoronoiVoidICAction.h"
#include "ReconVarICAction.h"
#include "RigidBodyMultiKernelAction.h"
#include "Tricrystal2CircleGrainsICAction.h"

/*
 * VectorPostprocessors
 */
#include "EulerAngleUpdaterCheck.h"
#include "FeatureVolumeFraction.h"
#include "FeatureVolumeVectorPostprocessor.h"
#include "GrainCentersPostprocessor.h"
#include "GrainForcesPostprocessor.h"
#include "GrainTextureVectorPostprocessor.h"

template <>
InputParameters
validParams<PhaseFieldApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

PhaseFieldApp::PhaseFieldApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  PhaseFieldApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PhaseFieldApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags();
  PhaseFieldApp::registerExecFlags();
}

PhaseFieldApp::~PhaseFieldApp() {}

// External entry point for dynamic application loading
extern "C" void
PhaseFieldApp__registerApps()
{
  PhaseFieldApp::registerApps();
}
void
PhaseFieldApp::registerApps()
{
  registerApp(PhaseFieldApp);
}

// External entry point for dynamic object registration
extern "C" void
PhaseFieldApp__registerObjects(Factory & factory)
{
  PhaseFieldApp::registerObjects(factory);
}
void
PhaseFieldApp::registerObjects(Factory & factory)
{
  registerKernel(ACGBPoly);
  registerKernel(ACGrGrElasticDrivingForce);
  registerKernel(ACGrGrMulti);
  registerKernel(ACGrGrPoly);
  registerKernel(ACInterface);
  registerKernel(ACMultiInterface);
  registerKernel(ACInterfaceKobayashi1);
  registerKernel(ACInterfaceKobayashi2);
  registerKernel(AllenCahnPFFracture);
  registerKernel(ACSEDGPoly);
  registerKernel(ACSwitching);
  registerKernel(AllenCahn);
  registerKernel(CahnHilliard);
  registerKernel(CahnHilliardAniso);
  registerKernel(CHBulkPFCTrad);
  registerKernel(CHCpldPFCTrad);
  registerKernel(CHInterface);
  registerKernel(CHInterfaceAniso);
  registerKernel(CHMath);
  registerKernel(CHPFCRFF);
  registerKernel(CHSplitChemicalPotential);
  registerKernel(CHSplitConcentration);
  registerKernel(CHSplitFlux);
  registerKernel(LaplacianSplit);
  registerKernel(CoefCoupledTimeDerivative);
  registerKernel(ConservedLangevinNoise);
  registerKernel(CoupledAllenCahn);
  registerKernel(CoupledSusceptibilityTimeDerivative);
  registerKernel(CoupledSwitchingTimeDerivative);
  registerKernel(GradientComponent);
  registerKernel(HHPFCRFF);
  registerKernel(KKSACBulkC);
  registerKernel(KKSACBulkF);
  registerKernel(KKSCHBulk);
  registerKernel(KKSMultiACBulkC);
  registerKernel(KKSMultiACBulkF);
  registerKernel(KKSMultiPhaseConcentration);
  registerKernel(KKSPhaseChemicalPotential);
  registerKernel(KKSPhaseConcentration);
  registerKernel(KKSSplitCHCRes);
  registerKernel(LangevinNoise);
  registerKernel(MaskedBodyForce);
  registerKernel(MatAnisoDiffusion);
  registerKernel(MatDiffusion);
  registerKernel(MatReaction);
  registerKernel(MultiGrainRigidBodyMotion);
  registerKernel(PFFracBulkRate);
  registerKernel(PFFracCoupledInterface);
  registerKernel(SimpleACInterface);
  registerKernel(SimpleCHInterface);
  registerKernel(SimpleCoupledACInterface);
  registerKernel(SimpleSplitCHWRes);
  registerKernel(SingleGrainRigidBodyMotion);
  registerKernel(SoretDiffusion);
  registerKernel(SplitCHMath);
  registerKernel(SplitCHParsed);
  registerKernel(SplitCHWRes);
  registerKernel(SplitCHWResAniso);
  registerKernel(SusceptibilityTimeDerivative);
  registerKernel(SwitchingFunctionConstraintEta);
  registerKernel(SwitchingFunctionConstraintLagrange);
  registerKernel(SwitchingFunctionPenalty);
  registerDeprecatedObjectName(LaplacianSplit, "CHSplitVar", "07/01/2017 00:00");

  registerInitialCondition(BimodalInverseSuperellipsoidsIC);
  registerInitialCondition(BimodalSuperellipsoidsIC);
  registerInitialCondition(ClosePackIC);
  registerInitialCondition(CrossIC);
  registerInitialCondition(HexPolycrystalIC);
  registerInitialCondition(LatticeSmoothCircleIC);
  registerInitialCondition(MultiBoundingBoxIC);
  registerInitialCondition(MultiSmoothCircleIC);
  registerInitialCondition(MultiSmoothSuperellipsoidIC);
  registerInitialCondition(PFCFreezingIC);
  registerInitialCondition(PolycrystalRandomIC);
  registerInitialCondition(PolycrystalReducedIC);
  registerInitialCondition(PolycrystalVoronoiVoidIC);
  registerInitialCondition(RampIC);
  registerInitialCondition(ReconPhaseVarIC);
  registerInitialCondition(ReconVarIC);
  registerInitialCondition(RndBoundingBoxIC);
  registerInitialCondition(RndSmoothCircleIC);
  registerInitialCondition(SmoothCircleIC);
  registerInitialCondition(SmoothSuperellipsoidIC);
  registerInitialCondition(SpecifiedSmoothCircleIC);
  registerInitialCondition(SpecifiedSmoothSuperellipsoidIC);
  registerInitialCondition(ThumbIC);
  registerInitialCondition(Tricrystal2CircleGrainsIC);
  registerInitialCondition(TricrystalTripleJunctionIC);

  registerBoundaryCondition(CahnHilliardAnisoFluxBC);
  registerBoundaryCondition(CahnHilliardFluxBC);

  registerInterfaceKernel(EqualGradientLagrangeInterface);
  registerInterfaceKernel(EqualGradientLagrangeMultiplier);
  registerInterfaceKernel(InterfaceDiffusionBoundaryTerm);
  registerInterfaceKernel(InterfaceDiffusionFluxMatch);

  registerMaterial(AsymmetricCrossTermBarrierFunctionMaterial);
  registerMaterial(BarrierFunctionMaterial);
  registerMaterial(CompositeMobilityTensor);
  registerMaterial(ComputePolycrystalElasticityTensor);
  registerMaterial(ConstantAnisotropicMobility);
  registerMaterial(CrossTermBarrierFunctionMaterial);
  registerMaterial(DeformedGrainMaterial);
  registerMaterial(DerivativeMultiPhaseMaterial);
  registerMaterial(DerivativeParsedMaterial);
  registerMaterial(DerivativeSumMaterial);
  registerMaterial(DerivativeTwoPhaseMaterial);
  registerMaterial(DiscreteNucleation);
  registerMaterial(ElasticEnergyMaterial);
  registerMaterial(ExternalForceDensityMaterial);
  registerMaterial(ForceDensityMaterial);
  registerMaterial(GBAnisotropy);
  registerMaterial(GBEvolution);
  registerMaterial(GBDependentAnisotropicTensor);
  registerMaterial(GBDependentDiffusivity);
  registerMaterial(GBWidthAnisotropy);
  registerMaterial(GrainAdvectionVelocity);
  registerMaterial(InterfaceOrientationMaterial);
  registerMaterial(KKSXeVacSolidMaterial);
  registerMaterial(MathEBFreeEnergy);
  registerMaterial(MathFreeEnergy);
  registerMaterial(MultiBarrierFunctionMaterial);
  registerMaterial(ParsedMaterial);
  registerMaterial(PFCRFFMaterial);
  registerMaterial(PFCTradMaterial);
  registerMaterial(PFFracBulkRateMaterial);
  registerMaterial(PFParamsPolyFreeEnergy);
  registerMaterial(PhaseNormalTensor);
  registerMaterial(PolynomialFreeEnergy);
  registerMaterial(RegularSolutionFreeEnergy);
  registerMaterial(StrainGradDispDerivatives);
  registerMaterial(SwitchingFunction3PhaseMaterial);
  registerMaterial(SwitchingFunctionMaterial);
  registerMaterial(SwitchingFunctionMultiPhaseMaterial);
  registerMaterial(ThirdPhaseSuppressionMaterial);
  registerMaterial(TimeStepMaterial);
  registerMaterial(VariableGradientMaterial);

  registerPostprocessor(FauxGrainTracker);
  registerPostprocessor(FeatureFloodCount);
  registerPostprocessor(FeatureVolumeFraction);
  registerPostprocessor(GrainBoundaryArea);
  registerPostprocessor(GrainTracker);
  registerPostprocessor(GrainTrackerElasticity);
  registerPostprocessor(PFCElementEnergyIntegral);

  registerAux(BndsCalcAux);
  registerAux(CrossTermGradientFreeEnergy);
  registerAux(EBSDReaderAvgDataAux);
  registerAux(EBSDReaderPointDataAux);
  registerAux(EulerAngleProvider2RGBAux);
  registerAux(EulerAngleVariables2RGBAux);
  registerAux(FeatureFloodCountAux);
  registerAux(GrainAdvectionAux);
  registerAux(KKSGlobalFreeEnergy);
  registerAux(KKSMultiFreeEnergy);
  registerAux(OutputEulerAngles);
  registerAux(PFCEnergyDensity);
  registerAux(PFCRFFEnergyDensity);
  registerAux(TotalFreeEnergy);

  registerUserObject(ComputeExternalGrainForceAndTorque);
  registerUserObject(ComputeGrainForceAndTorque);
  registerUserObject(ConservedMaskedNormalNoise);
  registerUserObject(ConservedMaskedUniformNoise);
  registerUserObject(ConservedNormalNoise);
  registerUserObject(ConservedUniformNoise);
  registerUserObject(ConstantGrainForceAndTorque);
  registerUserObject(DiscreteNucleationInserter);
  registerUserObject(DiscreteNucleationMap);
  registerUserObject(EBSDReader);
  registerUserObject(EulerAngleUpdater);
  registerUserObject(GrainForceAndTorqueSum);
  registerUserObject(MaskedGrainForceAndTorque);
  registerUserObject(RandomEulerAngleProvider);
  registerUserObject(SolutionRasterizer);

  registerVectorPostprocessor(EulerAngleUpdaterCheck);
  registerVectorPostprocessor(FeatureVolumeVectorPostprocessor);
  registerVectorPostprocessor(GrainForcesPostprocessor);
  registerVectorPostprocessor(GrainTextureVectorPostprocessor);

  registerMesh(EBSDMesh);
  registerMesh(MortarPeriodicMesh);
}

// External entry point for dynamic syntax association
extern "C" void
PhaseFieldApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  PhaseFieldApp::associateSyntax(syntax, action_factory);
}
void
PhaseFieldApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerSyntax("BicrystalBoundingBoxICAction", "ICs/PolycrystalICs/BicrystalBoundingBoxIC");
  registerSyntax("BicrystalCircleGrainICAction", "ICs/PolycrystalICs/BicrystalCircleGrainIC");
  registerSyntax("CHPFCRFFSplitKernelAction", "Kernels/CHPFCRFFSplitKernel");
  registerSyntax("CHPFCRFFSplitVariablesAction", "Variables/CHPFCRFFSplitVariables");
  registerSyntax("DisplacementGradientsAction", "Modules/PhaseField/DisplacementGradients");
  registerSyntax("EmptyAction", "ICs/PolycrystalICs"); // placeholder
  registerSyntax("EulerAngle2RGBAction", "Modules/PhaseField/EulerAngles2RGB");
  registerSyntax("HHPFCRFFSplitKernelAction", "Kernels/HHPFCRFFSplitKernel");
  registerSyntax("HHPFCRFFSplitVariablesAction", "Variables/HHPFCRFFSplitVariables");
  registerSyntax("MatVecRealGradAuxKernelAction", "AuxKernels/MatVecRealGradAuxKernel");
  registerSyntax("MaterialVectorAuxKernelAction", "AuxKernels/MaterialVectorAuxKernel");
  registerSyntax("MaterialVectorGradAuxKernelAction", "AuxKernels/MaterialVectorGradAuxKernel");
  registerSyntax("MortarPeriodicAction", "Modules/PhaseField/MortarPeriodicity/*");
  registerSyntax("MultiAuxVariablesAction", "AuxVariables/MultiAuxVariables");
  registerSyntax("PFCRFFKernelAction", "Kernels/PFCRFFKernel");
  registerSyntax("PFCRFFVariablesAction", "Variables/PFCRFFVariables");
  registerSyntax("PolycrystalElasticDrivingForceAction", "Kernels/PolycrystalElasticDrivingForce");
  registerSyntax("PolycrystalHexGrainICAction", "ICs/PolycrystalICs/PolycrystalHexGrainIC");
  registerSyntax("PolycrystalKernelAction", "Kernels/PolycrystalKernel");
  registerSyntax("PolycrystalRandomICAction", "ICs/PolycrystalICs/PolycrystalRandomIC");
  registerSyntax("PolycrystalStoredEnergyAction", "Kernels/PolycrystalStoredEnergy");
  registerSyntax("PolycrystalVariablesAction", "Variables/PolycrystalVariables");
  registerSyntax("PolycrystalVoronoiICAction", "ICs/PolycrystalICs/PolycrystalVoronoiIC");
  registerSyntax("PolycrystalVoronoiVoidICAction", "ICs/PolycrystalICs/PolycrystalVoronoiVoidIC");
  registerSyntax("ReconVarICAction", "ICs/PolycrystalICs/ReconVarIC");
  registerSyntax("RigidBodyMultiKernelAction", "Kernels/RigidBodyMultiKernel");
  registerSyntax("Tricrystal2CircleGrainsICAction", "ICs/PolycrystalICs/Tricrystal2CircleGrainsIC");

  registerAction(BicrystalBoundingBoxICAction, "add_ic");
  registerAction(BicrystalCircleGrainICAction, "add_ic");
  registerAction(CHPFCRFFSplitKernelAction, "add_kernel");
  registerAction(CHPFCRFFSplitVariablesAction, "add_variable");
  registerAction(DisplacementGradientsAction, "add_kernel");
  registerAction(DisplacementGradientsAction, "add_material");
  registerAction(DisplacementGradientsAction, "add_variable");
  registerAction(EulerAngle2RGBAction, "add_aux_kernel");
  registerAction(EulerAngle2RGBAction, "add_aux_variable");
  registerAction(HHPFCRFFSplitKernelAction, "add_kernel");
  registerAction(HHPFCRFFSplitVariablesAction, "add_variable");
  registerAction(MaterialVectorAuxKernelAction, "add_aux_kernel");
  registerAction(MaterialVectorGradAuxKernelAction, "add_aux_kernel");
  registerAction(MatVecRealGradAuxKernelAction, "add_aux_kernel");
  registerAction(MortarPeriodicAction, "add_constraint");
  registerAction(MortarPeriodicAction, "add_mortar_interface");
  registerAction(MortarPeriodicAction, "add_variable");
  registerAction(MultiAuxVariablesAction, "add_aux_variable");
  registerAction(PFCRFFKernelAction, "add_kernel");
  registerAction(PFCRFFVariablesAction, "add_variable");
  registerAction(PolycrystalElasticDrivingForceAction, "add_kernel");
  registerAction(PolycrystalHexGrainICAction, "add_ic");
  registerAction(PolycrystalKernelAction, "add_kernel");
  registerAction(PolycrystalRandomICAction, "add_ic");
  registerAction(PolycrystalStoredEnergyAction, "add_kernel");
  registerAction(PolycrystalVariablesAction, "add_variable");
  registerAction(PolycrystalVoronoiICAction, "add_ic");
  registerAction(PolycrystalVoronoiVoidICAction, "add_ic");
  registerAction(ReconVarICAction, "add_aux_variable");
  registerAction(ReconVarICAction, "add_ic");
  registerAction(RigidBodyMultiKernelAction, "add_kernel");
  registerAction(Tricrystal2CircleGrainsICAction, "add_ic");
}

void
PhaseFieldApp::registerExecFlags()
{
}
