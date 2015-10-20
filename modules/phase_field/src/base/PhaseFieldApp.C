/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PhaseFieldApp.h"
#include "Moose.h"
#include "AppFactory.h"

/*
 * Kernels
 */
#include "ACGBPoly.h"
#include "ACGrGrElasticDrivingForce.h"
#include "ACGrGrPoly.h"
#include "ACInterface.h"
#include "ACMultiInterface.h"
#include "ACParsed.h"
#include "ACInterfaceKobayashi1.h"
#include "ACInterfaceKobayashi2.h"
#include "CahnHilliard.h"
#include "CahnHilliardAniso.h"
#include "CHBulkPFCTrad.h"
#include "CHCpldPFCTrad.h"
#include "CHInterface.h"
#include "CHInterfaceAniso.h"
#include "CHMath.h"
#include "CHPFCRFF.h"
#include "CHSplitVar.h"
#include "CoefCoupledTimeDerivative.h"
#include "ConservedLangevinNoise.h"
#include "CoupledTimeDerivative.h"
#include "GradientComponent.h"
#include "HHPFCRFF.h"
#include "KKSACBulkC.h"
#include "KKSACBulkF.h"
#include "KKSCHBulk.h"
#include "KKSPhaseChemicalPotential.h"
#include "KKSPhaseConcentration.h"
#include "KKSSplitCHCRes.h"
#include "LangevinNoise.h"
#include "MaskedBodyForce.h"
#include "MatDiffusion.h"
#include "MultiGrainRigidBodyMotion.h"
#include "PFFracBulkRate.h"
#include "PFFracCoupledInterface.h"
#include "PFFracIntVar.h"
#include "SingleGrainRigidBodyMotion.h"
#include "SoretDiffusion.h"
#include "SplitCHMath.h"
#include "SplitCHParsed.h"
#include "SplitCHWRes.h"
#include "SplitCHWResAniso.h"
#include "SwitchingFunctionConstraintEta.h"
#include "SwitchingFunctionConstraintLagrange.h"
#include "SwitchingFunctionPenalty.h"

/*
 * Initial Conditions
 */
#include "ClosePackIC.h"
#include "CrossIC.h"
#include "HexPolycrystalIC.h"
#include "LatticeSmoothCircleIC.h"
#include "MultiSmoothCircleIC.h"
#include "PFCFreezingIC.h"
#include "PolycrystalRandomIC.h"
#include "PolycrystalReducedIC.h"
#include "ReconVarIC.h"
#include "RndBoundingBoxIC.h"
#include "RndSmoothCircleIC.h"
#include "SmoothCircleIC.h"
#include "SpecifiedSmoothCircleIC.h"
#include "ThumbIC.h"
#include "Tricrystal2CircleGrainsIC.h"

/*
 * Boundary Conditions
 */
#include "CahnHilliardFluxBC.h"

/*
 * Materials
 */
#include "BarrierFunctionMaterial.h"
#include "ComputePolycrystalElasticityTensor.h"
#include "ConstantAnisotropicMobility.h"
#include "DerivativeMultiPhaseMaterial.h"
#include "DerivativeParsedMaterial.h"
#include "DerivativeSumMaterial.h"
#include "DerivativeTwoPhaseMaterial.h"
#include "DiscreteNucleation.h"
#include "ElasticEnergyMaterial.h"
#include "ExternalForceDensityMaterial.h"
#include "ForceDensityMaterial.h"
#include "GBAnisotropy.h"
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
#include "PolynomialFreeEnergy.h"
#include "StrainGradDispDerivatives.h"
#include "SwitchingFunctionMaterial.h"
#include "CrossTermBarrierFunctionMaterial.h"

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
#include "FeatureFloodCountAux.h"
#include "KKSGlobalFreeEnergy.h"
#include "PFCEnergyDensity.h"
#include "PFCRFFEnergyDensity.h"
#include "TestEBSDAux.h"
#include "TotalFreeEnergy.h"
#include "OutputEulerAngles.h"

/*
 * Functions
 */
#include "ImageFunction.h"

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
#include "ImageMesh.h"

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
#include "PFCRFFKernelAction.h"
#include "PFCRFFVariablesAction.h"
#include "PolycrystalElasticDrivingForceAction.h"
#include "PolycrystalHexGrainICAction.h"
#include "PolycrystalKernelAction.h"
#include "PolycrystalRandomICAction.h"
#include "PolycrystalVariablesAction.h"
#include "PolycrystalVoronoiICAction.h"
#include "ReconVarICAction.h"
#include "Tricrystal2CircleGrainsICAction.h"

/*
 * VectorPostprocessors
 */
 #include "GrainCentersPostprocessor.h"
 #include "GrainForcesPostprocessor.h"


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
  registerKernel(ACParsed);
  registerKernel(ACInterfaceKobayashi1);
  registerKernel(ACInterfaceKobayashi2);
  registerKernel(CahnHilliard);
  registerKernel(CahnHilliardAniso);
  registerKernel(CHBulkPFCTrad);
  registerKernel(CHCpldPFCTrad);
  registerKernel(CHInterface);
  registerKernel(CHInterfaceAniso);
  registerKernel(CHMath);
  registerKernel(CHPFCRFF);
  registerKernel(CHSplitVar);
  registerKernel(CoefCoupledTimeDerivative);
  registerKernel(ConservedLangevinNoise);
  registerKernel(GradientComponent);
  registerKernel(HHPFCRFF);
  registerKernel(KKSACBulkC);
  registerKernel(KKSACBulkF);
  registerKernel(KKSCHBulk);
  registerKernel(KKSPhaseChemicalPotential);
  registerKernel(KKSPhaseConcentration);
  registerKernel(KKSSplitCHCRes);
  registerKernel(LangevinNoise);
  registerKernel(MaskedBodyForce);
  registerKernel(MatDiffusion);
  registerKernel(MultiGrainRigidBodyMotion);
  registerKernel(PFFracBulkRate);
  registerKernel(PFFracCoupledInterface);
  registerKernel(PFFracIntVar);
  registerKernel(SingleGrainRigidBodyMotion);
  registerKernel(SoretDiffusion);
  registerKernel(SplitCHMath);
  registerKernel(SplitCHParsed);
  registerKernel(SplitCHWRes);
  registerKernel(SplitCHWResAniso);
  registerKernel(SwitchingFunctionConstraintEta);
  registerKernel(SwitchingFunctionConstraintLagrange);
  registerKernel(SwitchingFunctionPenalty);
  registerDeprecatedObjectName(CahnHilliard, "CHParsed", "11/01/2015 00:00");
  registerDeprecatedObjectName(CoupledTimeDerivative, "CoupledImplicitEuler", "09/01/2015 00:00");

  registerInitialCondition(ClosePackIC);
  registerInitialCondition(CrossIC);
  registerInitialCondition(HexPolycrystalIC);
  registerInitialCondition(LatticeSmoothCircleIC);
  registerInitialCondition(MultiSmoothCircleIC);
  registerInitialCondition(PFCFreezingIC);
  registerInitialCondition(PolycrystalRandomIC);
  registerInitialCondition(PolycrystalReducedIC);
  registerInitialCondition(ReconVarIC);
  registerInitialCondition(RndBoundingBoxIC);
  registerInitialCondition(RndSmoothCircleIC);
  registerInitialCondition(SmoothCircleIC);
  registerInitialCondition(SpecifiedSmoothCircleIC);
  registerInitialCondition(ThumbIC);
  registerInitialCondition(Tricrystal2CircleGrainsIC);

  registerBoundaryCondition(CahnHilliardFluxBC);

  registerMaterial(BarrierFunctionMaterial);
  registerMaterial(ComputePolycrystalElasticityTensor);
  registerMaterial(ConstantAnisotropicMobility);
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
  registerMaterial(PolynomialFreeEnergy);
  registerMaterial(StrainGradDispDerivatives);
  registerMaterial(SwitchingFunctionMaterial);
  registerMaterial(CrossTermBarrierFunctionMaterial);
  registerDeprecatedObjectName(PFMobility, "PFMobility", "09/26/2015 00:00");

  registerPostprocessor(FeatureFloodCount);
  registerPostprocessor(GrainTracker);
  registerPostprocessor(FauxGrainTracker);
  registerPostprocessor(NodalVolumeFraction);
  registerPostprocessor(PFCElementEnergyIntegral);

  registerAux(BndsCalcAux);
  registerAux(CrossTermGradientFreeEnergy);
  registerAux(FeatureFloodCountAux);
  registerAux(KKSGlobalFreeEnergy);
  registerAux(PFCEnergyDensity);
  registerAux(PFCRFFEnergyDensity);
  registerAux(TestEBSDAux);
  registerAux(TotalFreeEnergy);
  registerAux(OutputEulerAngles);

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

  registerFunction(ImageFunction);

  registerVectorPostprocessor(GrainCentersPostprocessor);
  registerVectorPostprocessor(GrainForcesPostprocessor);

  registerMesh(EBSDMesh);
  registerMesh(ImageMesh);
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
  syntax.registerActionSyntax("PFCRFFKernelAction", "Kernels/PFCRFFKernel");
  syntax.registerActionSyntax("PFCRFFVariablesAction", "Variables/PFCRFFVariables");
  syntax.registerActionSyntax("PolycrystalElasticDrivingForceAction", "Kernels/PolycrystalElasticDrivingForce");
  syntax.registerActionSyntax("PolycrystalHexGrainICAction", "ICs/PolycrystalICs/PolycrystalHexGrainIC");
  syntax.registerActionSyntax("PolycrystalKernelAction", "Kernels/PolycrystalKernel");
  syntax.registerActionSyntax("PolycrystalRandomICAction", "ICs/PolycrystalICs/PolycrystalRandomIC");
  syntax.registerActionSyntax("PolycrystalVariablesAction", "Variables/PolycrystalVariables");
  syntax.registerActionSyntax("PolycrystalVoronoiICAction", "ICs/PolycrystalICs/PolycrystalVoronoiIC");
  syntax.registerActionSyntax("ReconVarICAction", "ICs/PolycrystalICs/ReconVarIC");
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
  registerAction(PFCRFFKernelAction, "add_kernel");
  registerAction(PFCRFFVariablesAction, "add_variable");
  registerAction(PolycrystalElasticDrivingForceAction, "add_kernel");
  registerAction(PolycrystalHexGrainICAction, "add_ic");
  registerAction(PolycrystalKernelAction, "add_kernel");
  registerAction(PolycrystalRandomICAction, "add_ic");
  registerAction(PolycrystalVariablesAction, "add_variable");
  registerAction(PolycrystalVoronoiICAction, "add_ic");
  registerAction(ReconVarICAction, "add_ic");
  registerAction(Tricrystal2CircleGrainsICAction, "add_ic");
}
