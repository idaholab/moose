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
#include "ACGrGrPoly.h"
#include "ACInterface.h"
#include "ACMultiInterface.h"
#include "ACParsed.h"
#include "CHInterface.h"
#include "CHMath.h"
#include "CHParsed.h"
#include "ConservedLangevinNoise.h"
#include "CoupledImplicitEuler.h"
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
#include "DerivativeMultiPhaseMaterial.h"
#include "DerivativeParsedMaterial.h"
#include "DerivativeSumMaterial.h"
#include "DerivativeTwoPhaseMaterial.h"
#include "ElasticEnergyMaterial.h"
#include "GBAnisotropy.h"
#include "GBEvolution.h"
#include "KKSXeVacSolidMaterial.h"
#include "LinearIsoElasticPFDamage.h"
#include "MathEBFreeEnergy.h"
#include "MathFreeEnergy.h"
#include "MultiBarrierFunctionMaterial.h"
#include "ParsedMaterial.h"
#include "PFFracBulkRateMaterial.h"
#include "PFMobility.h"
#include "PFParamsPolyFreeEnergy.h"
#include "PolynomialFreeEnergy.h"
#include "SwitchingFunctionMaterial.h"

/*
 * Postprocessors
 */
#include "GrainTracker.h"
#include "FeatureFloodCount.h"
#include "NodalVolumeFraction.h"

/*
 * AuxKernels
 */
#include "BndsCalcAux.h"
#include "CrossTermGradientFreeEnergy.h"
#include "KKSGlobalFreeEnergy.h"
#include "FeatureFloodCountAux.h"
#include "TotalFreeEnergy.h"

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

PhaseFieldApp::PhaseFieldApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
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
  registerApp(PhaseFieldApp);
}

// External entry point for dynamic object registration
extern "C" void PhaseFieldApp__registerObjects(Factory & factory) { PhaseFieldApp::registerObjects(factory); }
void
PhaseFieldApp::registerObjects(Factory & factory)
{
  registerKernel(ACGBPoly);
  registerKernel(ACGrGrPoly);
  registerKernel(ACInterface);
  registerKernel(ACMultiInterface);
  registerKernel(ACParsed);
  registerKernel(CHInterface);
  registerKernel(CHMath);
  registerKernel(CHParsed);
  registerKernel(ConservedLangevinNoise);
  registerKernel(CoupledImplicitEuler);
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
  registerKernel(SplitCHMath);
  registerKernel(SplitCHParsed);
  registerKernel(SplitCHWRes);
  registerKernel(SwitchingFunctionConstraintEta);
  registerKernel(SwitchingFunctionConstraintLagrange);
  registerKernel(SwitchingFunctionPenalty);

  registerInitialCondition(ClosePackIC);
  registerInitialCondition(CrossIC);
  registerInitialCondition(HexPolycrystalIC);
  registerInitialCondition(LatticeSmoothCircleIC);
  registerInitialCondition(MultiSmoothCircleIC);
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
  registerMaterial(DerivativeMultiPhaseMaterial);
  registerMaterial(DerivativeParsedMaterial);
  registerMaterial(DerivativeSumMaterial);
  registerMaterial(DerivativeTwoPhaseMaterial);
  registerMaterial(ElasticEnergyMaterial);
  registerMaterial(GBAnisotropy);
  registerMaterial(GBEvolution);
  registerMaterial(KKSXeVacSolidMaterial);
  registerMaterial(LinearIsoElasticPFDamage);
  registerMaterial(MathEBFreeEnergy);
  registerMaterial(MathFreeEnergy);
  registerMaterial(MultiBarrierFunctionMaterial);
  registerMaterial(ParsedMaterial);
  registerMaterial(PFFracBulkRateMaterial);
  registerMaterial(PFMobility);
  registerMaterial(PFParamsPolyFreeEnergy);
  registerMaterial(PolynomialFreeEnergy);
  registerMaterial(SwitchingFunctionMaterial);

  registerPostprocessor(FeatureFloodCount);
  registerPostprocessor(GrainTracker);
  registerPostprocessor(NodalVolumeFraction);

  registerAux(BndsCalcAux);
  registerAux(CrossTermGradientFreeEnergy);
  registerAux(KKSGlobalFreeEnergy);
  registerAux(FeatureFloodCountAux);
  registerAux(TotalFreeEnergy);

  registerUserObject(ConservedMaskedNormalNoise);
  registerUserObject(ConservedMaskedUniformNoise);
  registerUserObject(ConservedNormalNoise);
  registerUserObject(ConservedUniformNoise);
  registerUserObject(EBSDReader);
  registerUserObject(SolutionRasterizer);

  registerFunction(ImageFunction);

  registerMesh(EBSDMesh);
  registerMesh(ImageMesh);
}

// External entry point for dynamic syntax association
extern "C" void PhaseFieldApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { PhaseFieldApp::associateSyntax(syntax, action_factory); }
void
PhaseFieldApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  syntax.registerActionSyntax("PolycrystalKernelAction", "Kernels/PolycrystalKernel");
  syntax.registerActionSyntax("PolycrystalVariablesAction", "Variables/PolycrystalVariables");
  syntax.registerActionSyntax("EmptyAction", "ICs/PolycrystalICs");  // placeholder
  syntax.registerActionSyntax("BicrystalCircleGrainICAction", "ICs/PolycrystalICs/BicrystalCircleGrainIC");
  syntax.registerActionSyntax("BicrystalBoundingBoxICAction", "ICs/PolycrystalICs/BicrystalBoundingBoxIC");
  syntax.registerActionSyntax("Tricrystal2CircleGrainsICAction", "ICs/PolycrystalICs/Tricrystal2CircleGrainsIC");
  syntax.registerActionSyntax("PolycrystalHexGrainICAction", "ICs/PolycrystalICs/PolycrystalHexGrainIC");
  syntax.registerActionSyntax("PolycrystalVoronoiICAction", "ICs/PolycrystalICs/PolycrystalVoronoiIC");
  syntax.registerActionSyntax("PolycrystalRandomICAction", "ICs/PolycrystalICs/PolycrystalRandomIC");
  syntax.registerActionSyntax("ReconVarICAction", "ICs/PolycrystalICs/ReconVarIC");

  registerAction(PolycrystalKernelAction, "add_kernel");
  registerAction(PolycrystalVariablesAction, "add_variable");
  registerAction(BicrystalCircleGrainICAction, "add_ic");
  registerAction(BicrystalBoundingBoxICAction, "add_ic");
  registerAction(Tricrystal2CircleGrainsICAction, "add_ic");
  registerAction(PolycrystalHexGrainICAction, "add_ic");
  registerAction(PolycrystalVoronoiICAction, "add_ic");
  registerAction(PolycrystalRandomICAction, "add_ic");
  registerAction(ReconVarICAction, "add_ic");
}
