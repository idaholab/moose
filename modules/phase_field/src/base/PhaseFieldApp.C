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
#include "ACGrGrPoly.h"
#include "ACInterface.h"
#include "ACMultiInterface.h"
#include "ACInterfaceKobayashi1.h"
#include "ACInterfaceKobayashi2.h"
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
#include "CHSplitVar.h"
#include "CoefCoupledTimeDerivative.h"
#include "ConservedLangevinNoise.h"
#include "CoupledAllenCahn.h"
#include "CoupledSusceptibilityTimeDerivative.h"
#include "GradientComponent.h"
#include "HHPFCRFF.h"
#include "KKSACBulkC.h"
#include "KKSACBulkF.h"
#include "KKSCHBulk.h"
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
#include "PFFracIntVar.h"
#include "SimpleACInterface.h"
#include "SimpleCHInterface.h"
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

/*
 * Boundary Conditions
 */
#include "CahnHilliardAnisoFluxBC.h"
#include "CahnHilliardFluxBC.h"

/*
 * Materials
 */
#include "AsymmetricCrossTermBarrierFunctionMaterial.h"
#include "BarrierFunctionMaterial.h"
#include "CompositeMobilityTensor.h"
#include "ComputePolycrystalElasticityTensor.h"
#include "ConstantAnisotropicMobility.h"
#include "CrossTermBarrierFunctionMaterial.h"
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
#include "ThirdPhaseSuppressionMaterial.h"

/*
 * Postprocessors
 */
#include "FeatureFloodCount.h"
#include "GrainTracker.h"
#include "FauxGrainTracker.h"
#include "NodalVolumeFraction.h"
#include "PFCElementEnergyIntegral.h"

/*
 * AuxKernels
 */
#include "BndsCalcAux.h"
#include "CrossTermGradientFreeEnergy.h"
#include "Euler2RGBAux.h"
#include "FeatureFloodCountAux.h"
#include "KKSGlobalFreeEnergy.h"
#include "PFCEnergyDensity.h"
#include "PFCRFFEnergyDensity.h"
#include "EBSDReaderAvgDataAux.h"
#include "EBSDReaderPointDataAux.h"
#include "TotalFreeEnergy.h"
#include "OutputEulerAngles.h"
#include "OutputRGB.h"

/*
 * Functions
 */

/*
 * User Objects
 */
#include "ComputeGrainCenterUserObject.h"
#include "ComputeGrainForceAndTorque.h"
#include "ConservedMaskedNormalNoise.h"
#include "ConservedMaskedUniformNoise.h"
#include "ConservedNormalNoise.h"
#include "ConservedUniformNoise.h"
#include "ConstantGrainForceAndTorque.h"
#include "DiscreteNucleationInserter.h"
#include "DiscreteNucleationMap.h"
#include "GrainForceAndTorqueSum.h"
#include "MaskedGrainForceAndTorque.h"

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
#include "DisplacementGradientsAction.h"
#include "CHPFCRFFSplitVariablesAction.h"
#include "HHPFCRFFSplitKernelAction.h"
#include "HHPFCRFFSplitVariablesAction.h"
#include "MatVecRealGradAuxKernelAction.h"
#include "MultiAuxVariablesAction.h"
#include "PFCRFFKernelAction.h"
#include "PFCRFFVariablesAction.h"
#include "PolycrystalElasticDrivingForceAction.h"
#include "PolycrystalHexGrainICAction.h"
#include "PolycrystalKernelAction.h"
#include "PolycrystalRandomICAction.h"
#include "PolycrystalVariablesAction.h"
#include "PolycrystalVoronoiICAction.h"
#include "ReconVarICAction.h"
#include "RigidBodyMultiKernelAction.h"
#include "Tricrystal2CircleGrainsICAction.h"

/*
 * VectorPostprocessors
 */
 #include "GrainCentersPostprocessor.h"
 #include "GrainForcesPostprocessor.h"
 #include "GrainTextureVectorPostprocessor.h"


template<>
InputParameters validParams<PhaseFieldApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}

PhaseFieldApp::PhaseFieldApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  PhaseFieldApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PhaseFieldApp::associateSyntax(_syntax, _action_factory);
}

PhaseFieldApp::~PhaseFieldApp()
{
}

// External entry point for dynamic application loading
extern "C" void PhaseFieldApp__registerApps() { PhaseFieldApp::registerApps(); }
void
PhaseFieldApp::registerApps()
{
  registerApp(PhaseFieldApp);
}

// External entry point for dynamic object registration
extern "C" void PhaseFieldApp__registerObjects(Factory & factory) { PhaseFieldApp::registerObjects(factory); }
void
PhaseFieldApp::registerObjects(Factory & factory)
{
  registerKernel(ACGBPoly);
  registerKernel(ACGrGrElasticDrivingForce);
  registerKernel(ACGrGrPoly);
  registerKernel(ACInterface);
  registerKernel(ACMultiInterface);
  registerKernel(ACInterfaceKobayashi1);
  registerKernel(ACInterfaceKobayashi2);
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
  registerKernel(CHSplitVar);
  registerKernel(CoefCoupledTimeDerivative);
  registerKernel(ConservedLangevinNoise);
  registerKernel(CoupledAllenCahn);
  registerKernel(CoupledSusceptibilityTimeDerivative);
  registerKernel(GradientComponent);
  registerKernel(HHPFCRFF);
  registerKernel(KKSACBulkC);
  registerKernel(KKSACBulkF);
  registerKernel(KKSCHBulk);
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
  registerKernel(PFFracIntVar);
  registerKernel(SimpleACInterface);
  registerKernel(SimpleCHInterface);
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
  registerDeprecatedObjectName(AllenCahn, "ACParsed", "15/04/2016 00:00");
  registerDeprecatedObjectName(CahnHilliard, "CHParsed", "11/01/2015 00:00");

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

  registerBoundaryCondition(CahnHilliardAnisoFluxBC);
  registerBoundaryCondition(CahnHilliardFluxBC);

  registerMaterial(AsymmetricCrossTermBarrierFunctionMaterial);
  registerMaterial(BarrierFunctionMaterial);
  registerMaterial(CompositeMobilityTensor);
  registerMaterial(ComputePolycrystalElasticityTensor);
  registerMaterial(ConstantAnisotropicMobility);
  registerMaterial(CrossTermBarrierFunctionMaterial);
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
  registerMaterial(ThirdPhaseSuppressionMaterial);

  registerPostprocessor(FeatureFloodCount);
  registerPostprocessor(GrainTracker);
  registerPostprocessor(FauxGrainTracker);
  registerPostprocessor(NodalVolumeFraction);
  registerPostprocessor(PFCElementEnergyIntegral);

  registerAux(BndsCalcAux);
  registerAux(CrossTermGradientFreeEnergy);
  registerAux(Euler2RGBAux);
  registerAux(FeatureFloodCountAux);
  registerAux(KKSGlobalFreeEnergy);
  registerAux(PFCEnergyDensity);
  registerAux(PFCRFFEnergyDensity);
  registerAux(EBSDReaderAvgDataAux);
  registerAux(EBSDReaderPointDataAux);
  registerAux(TotalFreeEnergy);
  registerAux(OutputEulerAngles);
  registerAux(OutputRGB);

  registerUserObject(ComputeGrainCenterUserObject);
  registerUserObject(ComputeGrainForceAndTorque);
  registerUserObject(ConservedMaskedNormalNoise);
  registerUserObject(ConservedMaskedUniformNoise);
  registerUserObject(ConservedNormalNoise);
  registerUserObject(ConservedUniformNoise);
  registerUserObject(ConstantGrainForceAndTorque);
  registerUserObject(DiscreteNucleationInserter);
  registerUserObject(DiscreteNucleationMap);
  registerUserObject(GrainForceAndTorqueSum);
  registerUserObject(MaskedGrainForceAndTorque);

  registerUserObject(EBSDReader);
  registerUserObject(SolutionRasterizer);

  registerVectorPostprocessor(GrainCentersPostprocessor);
  registerVectorPostprocessor(GrainForcesPostprocessor);
  registerVectorPostprocessor(GrainTextureVectorPostprocessor);

  registerMesh(EBSDMesh);
  registerMesh(MortarPeriodicMesh);
}

// External entry point for dynamic syntax association
extern "C" void PhaseFieldApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { PhaseFieldApp::associateSyntax(syntax, action_factory); }
void
PhaseFieldApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  syntax.registerActionSyntax("BicrystalBoundingBoxICAction", "ICs/PolycrystalICs/BicrystalBoundingBoxIC");
  syntax.registerActionSyntax("BicrystalCircleGrainICAction", "ICs/PolycrystalICs/BicrystalCircleGrainIC");
  syntax.registerActionSyntax("CHPFCRFFSplitKernelAction", "Kernels/CHPFCRFFSplitKernel");
  syntax.registerActionSyntax("CHPFCRFFSplitVariablesAction", "Variables/CHPFCRFFSplitVariables");
  syntax.registerActionSyntax("DisplacementGradientsAction", "Modules/PhaseField/DisplacementGradients");
  syntax.registerActionSyntax("EmptyAction", "ICs/PolycrystalICs");  // placeholder
  syntax.registerActionSyntax("HHPFCRFFSplitKernelAction", "Kernels/HHPFCRFFSplitKernel");
  syntax.registerActionSyntax("HHPFCRFFSplitVariablesAction", "Variables/HHPFCRFFSplitVariables");
  syntax.registerActionSyntax("MatVecRealGradAuxKernelAction", "AuxKernels/MatVecRealGradAuxKernel");
  syntax.registerActionSyntax("MultiAuxVariablesAction", "AuxVariables/MultiAuxVariables");
  syntax.registerActionSyntax("PFCRFFKernelAction", "Kernels/PFCRFFKernel");
  syntax.registerActionSyntax("PFCRFFVariablesAction", "Variables/PFCRFFVariables");
  syntax.registerActionSyntax("PolycrystalElasticDrivingForceAction", "Kernels/PolycrystalElasticDrivingForce");
  syntax.registerActionSyntax("PolycrystalHexGrainICAction", "ICs/PolycrystalICs/PolycrystalHexGrainIC");
  syntax.registerActionSyntax("PolycrystalKernelAction", "Kernels/PolycrystalKernel");
  syntax.registerActionSyntax("PolycrystalRandomICAction", "ICs/PolycrystalICs/PolycrystalRandomIC");
  syntax.registerActionSyntax("PolycrystalVariablesAction", "Variables/PolycrystalVariables");
  syntax.registerActionSyntax("PolycrystalVoronoiICAction", "ICs/PolycrystalICs/PolycrystalVoronoiIC");
  syntax.registerActionSyntax("ReconVarICAction", "ICs/PolycrystalICs/ReconVarIC");
  syntax.registerActionSyntax("RigidBodyMultiKernelAction", "Kernels/RigidBodyMultiKernel");
  syntax.registerActionSyntax("Tricrystal2CircleGrainsICAction", "ICs/PolycrystalICs/Tricrystal2CircleGrainsIC");

  registerAction(BicrystalBoundingBoxICAction, "add_ic");
  registerAction(BicrystalCircleGrainICAction, "add_ic");
  registerAction(CHPFCRFFSplitKernelAction, "add_kernel");
  registerAction(CHPFCRFFSplitVariablesAction, "add_variable");
  registerAction(DisplacementGradientsAction, "add_variable");
  registerAction(DisplacementGradientsAction, "add_material");
  registerAction(DisplacementGradientsAction, "add_kernel");
  registerAction(HHPFCRFFSplitKernelAction, "add_kernel");
  registerAction(HHPFCRFFSplitVariablesAction, "add_variable");
  registerAction(MatVecRealGradAuxKernelAction, "add_aux_kernel");
  registerAction(MultiAuxVariablesAction, "add_aux_variable");
  registerAction(PFCRFFKernelAction, "add_kernel");
  registerAction(PFCRFFVariablesAction, "add_variable");
  registerAction(PolycrystalElasticDrivingForceAction, "add_kernel");
  registerAction(PolycrystalHexGrainICAction, "add_ic");
  registerAction(PolycrystalKernelAction, "add_kernel");
  registerAction(PolycrystalRandomICAction, "add_ic");
  registerAction(PolycrystalVariablesAction, "add_variable");
  registerAction(PolycrystalVoronoiICAction, "add_ic");
  registerAction(ReconVarICAction, "add_ic");
  registerAction(RigidBodyMultiKernelAction, "add_kernel");
  registerAction(Tricrystal2CircleGrainsICAction, "add_ic");
}
