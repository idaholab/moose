#include "PhaseFieldApp.h"
#include "Moose.h"
#include "AppFactory.h"

#include "MatDiffusion.h"
#include "ACInterface.h"
#include "CHMath.h"
#include "CHParsed.h"
#include "CHInterface.h"
#include "SplitCHWRes.h"
#include "SplitCHMath.h"
#include "SplitCHParsed.h"
#include "CoupledImplicitEuler.h"
#include "CrossIC.h"
#include "SmoothCircleIC.h"
#include "RndSmoothCircleIC.h"
#include "MultiSmoothCircleIC.h"
#include "LatticeSmoothCircleIC.h"
#include "SpecifiedSmoothCircleIC.h"
#include "RndBoundingBoxIC.h"
#include "PFMobility.h"
#include "NodalFloodCount.h"
#include "NodalFloodCountAux.h"
#include "NodalVolumeFraction.h"
#include "BndsCalcAux.h"
#include "ACGrGrPoly.h"
#include "ACGBPoly.h"
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
//#include "SPPARKSUserObject.h"
//#include "SPPARKSAux.h"

template<>
InputParameters validParams<PhaseFieldApp>()
{
  InputParameters params = validParams<MooseApp>();
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
  registerKernel(CHMath);
  registerKernel(CHParsed);
  registerKernel(CHInterface);
  registerKernel(SplitCHWRes);
  registerKernel(SplitCHMath);
  registerKernel(SplitCHParsed);
  registerKernel(CoupledImplicitEuler);
  registerKernel(ACGrGrPoly);
  registerKernel(ACGBPoly);
  registerInitialCondition(CrossIC);
  registerInitialCondition(SmoothCircleIC);
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
  registerUserObject(NodalFloodCount);
  registerAux(NodalFloodCountAux);
  registerAux(BndsCalcAux);
  // registerAux(SPPARKSAux);
  registerUserObject(NodalVolumeFraction);
  // registerUserObject(SPPARKSUserObject);
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
