/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PhaseFieldApp.h"
#include "Moose.h"
#include "AppFactory.h"

#include "MatDiffusion.h"
#include "ACInterface.h"
#include "ACMultiInterface.h"
#include "CHMath.h"
#include "CHParsed.h"
#include "CHInterface.h"
#include "SplitCHWRes.h"
#include "SplitCHMath.h"
#include "SplitCHParsed.h"
#include "CoupledImplicitEuler.h"
#include "SwitchingFunctionConstraintLagrange.h"
#include "SwitchingFunctionConstraintEta.h"
#include "SwitchingFunctionPenalty.h"
#include "CrossIC.h"
#include "SmoothCircleIC.h"
#include "ClosePackIC.h"
#include "RndSmoothCircleIC.h"
#include "MultiSmoothCircleIC.h"
#include "LatticeSmoothCircleIC.h"
#include "SpecifiedSmoothCircleIC.h"
#include "RndBoundingBoxIC.h"
#include "PFMobility.h"
#include "ParsedMaterial.h"
#include "DerivativeParsedMaterial.h"
#include "DerivativeSumMaterial.h"
#include "DerivativeTwoPhaseMaterial.h"
#include "DerivativeMultiPhaseMaterial.h"
#include "BarrierFunctionMaterial.h"
#include "MultiBarrierFunctionMaterial.h"
#include "SwitchingFunctionMaterial.h"
#include "ElasticEnergyMaterial.h"
#include "MathFreeEnergy.h"
#include "GBAnisotropy.h"
#include "NodalFloodCount.h"
#include "NodalFloodCountAux.h"
#include "NodalVolumeFraction.h"
#include "BndsCalcAux.h"
#include "TotalFreeEnergy.h"
#include "CrossTermGradientFreeEnergy.h"
#include "ACGrGrPoly.h"
#include "ACGBPoly.h"
#include "ACParsed.h"
#include "GBEvolution.h"
#include "HexPolycrystalIC.h"
#include "PolycrystalRandomIC.h"
#include "PolycrystalReducedIC.h"
#include "ThumbIC.h"
#include "Tricrystal2CircleGrainsIC.h"
#include "PolycrystalVariablesAction.h"
#include "PolycrystalKernelAction.h"
#include "BicrystalCircleGrainICAction.h"
#include "BicrystalBoundingBoxICAction.h"
#include "Tricrystal2CircleGrainsICAction.h"
#include "PolycrystalHexGrainICAction.h"
#include "PolycrystalVoronoiICAction.h"
#include "PolycrystalRandomICAction.h"
#ifdef LIBMESH_HAVE_VTK
#include "ImageFunction.h"
#endif
#include "SolutionRasterizer.h"
#include "ImageMesh.h"
#include "MaskedBodyForce.h"
#include "LangevinNoise.h"
#include "ConservedLangevinNoise.h"

#include "PFFracBulkRate.h"
#include "PFFracIntVar.h"
#include "PFFracCoupledInterface.h"
#include "LinearIsoElasticPFDamage.h"
#include "PFFracBulkRateMaterial.h"

#include "ConservedUniformNoise.h"
#include "ConservedNormalNoise.h"
#include "ConservedMaskedUniformNoise.h"
#include "ConservedMaskedNormalNoise.h"

//#include "SPPARKSUserObject.h"
//#include "SPPARKSAux.h"

template<>
InputParameters validParams<PhaseFieldApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = true;
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

extern "C" void PhaseFieldApp__registerApps() { PhaseFieldApp::registerApps(); }
void
PhaseFieldApp::registerApps()
{
  registerApp(PhaseFieldApp);
}

void
PhaseFieldApp::registerObjects(Factory & factory)
{
  registerKernel(MatDiffusion);
  registerKernel(ACInterface);
  registerKernel(ACMultiInterface);
  registerKernel(CHMath);
  registerKernel(CHParsed);
  registerKernel(CHInterface);
  registerKernel(SplitCHWRes);
  registerKernel(SplitCHMath);
  registerKernel(SplitCHParsed);
  registerKernel(CoupledImplicitEuler);
  registerKernel(ACGrGrPoly);
  registerKernel(ACGBPoly);
  registerKernel(ACParsed);
  registerKernel(MaskedBodyForce);
  registerKernel(SwitchingFunctionConstraintLagrange);
  registerKernel(SwitchingFunctionConstraintEta);
  registerKernel(SwitchingFunctionPenalty);
  registerKernel(PFFracBulkRate);
  registerKernel(PFFracIntVar);
  registerKernel(PFFracCoupledInterface);
  registerKernel(LangevinNoise);
  registerKernel(ConservedLangevinNoise);

  registerInitialCondition(CrossIC);
  registerInitialCondition(SmoothCircleIC);
  registerInitialCondition(ClosePackIC);
  registerInitialCondition(RndSmoothCircleIC);
  registerInitialCondition(MultiSmoothCircleIC);
  registerInitialCondition(LatticeSmoothCircleIC);
  registerInitialCondition(SpecifiedSmoothCircleIC);
  registerInitialCondition(RndBoundingBoxIC);
  registerInitialCondition(HexPolycrystalIC);
  registerInitialCondition(PolycrystalRandomIC);
  registerInitialCondition(PolycrystalReducedIC);
  registerInitialCondition(ThumbIC);
  registerInitialCondition(Tricrystal2CircleGrainsIC);

  registerMaterial(PFMobility);
  registerMaterial(GBEvolution);
  registerMaterial(ParsedMaterial);
  registerMaterial(DerivativeParsedMaterial);
  registerMaterial(DerivativeSumMaterial);
  registerMaterial(DerivativeTwoPhaseMaterial);
  registerMaterial(DerivativeMultiPhaseMaterial);
  registerMaterial(BarrierFunctionMaterial);
  registerMaterial(MultiBarrierFunctionMaterial);
  registerMaterial(SwitchingFunctionMaterial);
  registerMaterial(ElasticEnergyMaterial);
  registerMaterial(MathFreeEnergy);
  registerMaterial(GBAnisotropy);
  registerMaterial(LinearIsoElasticPFDamage);
  registerMaterial(PFFracBulkRateMaterial);

  registerAux(NodalFloodCountAux);
  registerAux(BndsCalcAux);
  registerAux(TotalFreeEnergy);
  registerAux(CrossTermGradientFreeEnergy);
  // registerAux(SPPARKSAux);

  registerUserObject(NodalFloodCount);
  registerUserObject(NodalVolumeFraction);
  registerUserObject(SolutionRasterizer);
  registerUserObject(ConservedUniformNoise);
  registerUserObject(ConservedNormalNoise);
  registerUserObject(ConservedMaskedUniformNoise);
  registerUserObject(ConservedMaskedNormalNoise);

  // registerUserObject(SPPARKSUserObject);

#ifdef LIBMESH_HAVE_VTK
  registerFunction(ImageFunction);
#endif

  registerMesh(ImageMesh);
}

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

  registerAction(PolycrystalKernelAction, "add_kernel");
  registerAction(PolycrystalVariablesAction, "add_variable");
  registerAction(BicrystalCircleGrainICAction, "add_ic");
  registerAction(BicrystalBoundingBoxICAction, "add_ic");
  registerAction(Tricrystal2CircleGrainsICAction, "add_ic");
  registerAction(PolycrystalHexGrainICAction, "add_ic");
  registerAction(PolycrystalVoronoiICAction, "add_ic");
  registerAction(PolycrystalRandomICAction, "add_ic");
}
