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
#include "CHBulkPFCTrad.h"
#include "CHCpldPFCTrad.h"
#include "CHInterface.h"
#include "CHMath.h"
#include "CHParsed.h"
#include "CHPFCRFF.h"
#include "CHSplitVar.h"
#include "ConservedLangevinNoise.h"
#include "CoupledTimeDerivative.h"
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
#include "PFFracBulkRate.h"
#include "PFFracCoupledInterface.h"
#include "PFFracIntVar.h"
#include "SoretDiffusion.h"
#include "SplitCHMath.h"
#include "SplitCHParsed.h"
#include "SplitCHWRes.h"
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
 * Materials
 */
#include "BarrierFunctionMaterial.h"
#include "ComputePolycrystalElasticityTensor.h"
#include "DerivativeMultiPhaseMaterial.h"
#include "DerivativeParsedMaterial.h"
#include "DerivativeSumMaterial.h"
#include "DerivativeTwoPhaseMaterial.h"
#include "DiscreteNucleation.h"
#include "ElasticEnergyMaterial.h"
#include "GBAnisotropy.h"
#include "GBEvolution.h"
#include "KKSXeVacSolidMaterial.h"
#include "LinearIsoElasticPFDamage.h"
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
#include "SwitchingFunctionMaterial.h"
#include "CrossTermBarrierFunctionMaterial.h"

/*
 * Postprocessors
 */
#include "FeatureFloodCount.h"
#include "GrainTracker.h"
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
#include "TotalFreeEnergy.h"
#include "OutputEulerAngles.h"

/*
 * Functions
 */
#include "ImageFunction.h"

/*
 * User Objects
 */
#include "ConservedMaskedNormalNoise.h"
#include "ConservedMaskedUniformNoise.h"
#include "ConservedNormalNoise.h"
#include "ConservedUniformNoise.h"
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
  srand(processor_id());

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
#undef  registerApp
#define registerApp(name) AppFactory::instance().reg<name>(#name)

  registerApp(PhaseFieldApp);


#undef  registerApp
#define registerApp(name) AppFactory::instance().regLegacy<name>(#name)
}

// External entry point for dynamic object registration
extern "C" void PhaseFieldApp__registerObjects(Factory & factory) { PhaseFieldApp::registerObjects(factory); }
void
PhaseFieldApp::registerObjects(Factory & factory)
{
#undef registerObject
#define registerObject(name) factory.reg<name>(stringifyName(name))
#undef registerDeprecatedObjectName
#define registerDeprecatedObjectName(obj, name, time) factory.regReplaced<obj>(stringifyName(obj), name, time)


  registerKernel(ACGBPoly);
  registerKernel(ACGrGrElasticDrivingForce);
  registerKernel(ACGrGrPoly);
  registerKernel(ACInterface);
  registerKernel(ACMultiInterface);
  registerKernel(ACParsed);
  registerKernel(CHBulkPFCTrad);
  registerKernel(CHCpldPFCTrad);
  registerKernel(CHInterface);
  registerKernel(CHMath);
  registerKernel(CHParsed);
  registerKernel(CHPFCRFF);
  registerKernel(CHSplitVar);
  registerKernel(ConservedLangevinNoise);
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
  registerKernel(PFFracBulkRate);
  registerKernel(PFFracCoupledInterface);
  registerKernel(PFFracIntVar);
  registerKernel(SoretDiffusion);
  registerKernel(SplitCHMath);
  registerKernel(SplitCHParsed);
  registerKernel(SplitCHWRes);
  registerKernel(SwitchingFunctionConstraintEta);
  registerKernel(SwitchingFunctionConstraintLagrange);
  registerKernel(SwitchingFunctionPenalty);
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

  registerMaterial(BarrierFunctionMaterial);
  registerMaterial(ComputePolycrystalElasticityTensor);
  registerMaterial(DerivativeMultiPhaseMaterial);
  registerMaterial(DerivativeParsedMaterial);
  registerMaterial(DerivativeSumMaterial);
  registerMaterial(DerivativeTwoPhaseMaterial);
  registerMaterial(DiscreteNucleation);
  registerMaterial(ElasticEnergyMaterial);
  registerMaterial(GBAnisotropy);
  registerMaterial(GBEvolution);
  registerMaterial(KKSXeVacSolidMaterial);
  registerMaterial(LinearIsoElasticPFDamage);
  registerMaterial(MathEBFreeEnergy);
  registerMaterial(MathFreeEnergy);
  registerMaterial(MultiBarrierFunctionMaterial);
  registerMaterial(ParsedMaterial);
  registerMaterial(PFCRFFMaterial);
  registerMaterial(PFCTradMaterial);
  registerMaterial(PFFracBulkRateMaterial);
  registerMaterial(PFMobility);
  registerMaterial(PFParamsPolyFreeEnergy);
  registerMaterial(PolynomialFreeEnergy);
  registerMaterial(SwitchingFunctionMaterial);
  registerMaterial(CrossTermBarrierFunctionMaterial);

  registerPostprocessor(FeatureFloodCount);
  registerPostprocessor(GrainTracker);
  registerPostprocessor(NodalVolumeFraction);
  registerPostprocessor(PFCElementEnergyIntegral);

  registerAux(BndsCalcAux);
  registerAux(CrossTermGradientFreeEnergy);
  registerAux(FeatureFloodCountAux);
  registerAux(KKSGlobalFreeEnergy);
  registerAux(PFCEnergyDensity);
  registerAux(PFCRFFEnergyDensity);
  registerAux(TotalFreeEnergy);
  registerAux(OutputEulerAngles);

  registerUserObject(ConservedMaskedNormalNoise);
  registerUserObject(ConservedMaskedUniformNoise);
  registerUserObject(ConservedNormalNoise);
  registerUserObject(ConservedUniformNoise);
  registerUserObject(EBSDReader);
  registerUserObject(SolutionRasterizer);

  registerFunction(ImageFunction);

  registerMesh(EBSDMesh);
  registerMesh(ImageMesh);

#undef registerDeprecatedObjectName
#define registerDeprecatedObjectName(obj, name, time) factory.regLegacyReplaced<obj>(stringifyName(obj), name, time)
#undef registerObject
#define registerObject(name) factory.regLegacy<name>(stringifyName(name))

}

// External entry point for dynamic syntax association
extern "C" void PhaseFieldApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { PhaseFieldApp::associateSyntax(syntax, action_factory); }
void
PhaseFieldApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
#undef registerAction
#define registerAction(tplt, action) action_factory.reg<tplt>(stringifyName(tplt), action)

  syntax.registerActionSyntax("BicrystalBoundingBoxICAction", "ICs/PolycrystalICs/BicrystalBoundingBoxIC");
  syntax.registerActionSyntax("BicrystalCircleGrainICAction", "ICs/PolycrystalICs/BicrystalCircleGrainIC");
  syntax.registerActionSyntax("CHPFCRFFSplitKernelAction", "Kernels/CHPFCRFFSplitKernel");
  syntax.registerActionSyntax("CHPFCRFFSplitVariablesAction", "Variables/CHPFCRFFSplitVariables");
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

#undef registerAction
#define registerAction(tplt, action) action_factory.regLegacy<tplt>(stringifyName(tplt), action)
}


// DEPRECATED CONSTRUCTOR
PhaseFieldApp::PhaseFieldApp(const std::string & deprecated_name, InputParameters parameters) :
    MooseApp(deprecated_name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  PhaseFieldApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PhaseFieldApp::associateSyntax(_syntax, _action_factory);
}
