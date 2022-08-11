//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
PhaseFieldApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;

  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("PhaseFieldApp");

PhaseFieldApp::PhaseFieldApp(const InputParameters & parameters) : MooseApp(parameters)
{
  PhaseFieldApp::registerAll(_factory, _action_factory, _syntax);
}

PhaseFieldApp::~PhaseFieldApp() {}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerSyntax("BicrystalBoundingBoxICAction", "ICs/PolycrystalICs/BicrystalBoundingBoxIC");
  registerSyntax("BicrystalCircleGrainICAction", "ICs/PolycrystalICs/BicrystalCircleGrainIC");
  registerSyntax("CHPFCRFFSplitKernelAction", "Kernels/CHPFCRFFSplitKernel");
  registerSyntax("CHPFCRFFSplitVariablesAction", "Variables/CHPFCRFFSplitVariables");
  registerSyntax("ConservedAction", "Modules/PhaseField/Conserved/*");
  registerSyntax("DisplacementGradientsAction", "Modules/PhaseField/DisplacementGradients");
  registerSyntax("EmptyAction", "ICs/PolycrystalICs"); // placeholder
  registerSyntax("EulerAngle2RGBAction", "Modules/PhaseField/EulerAngles2RGB");
  registerSyntax("GrainGrowthAction", "Modules/PhaseField/GrainGrowth");
  registerSyntax("HHPFCRFFSplitKernelAction", "Kernels/HHPFCRFFSplitKernel");
  registerSyntax("HHPFCRFFSplitVariablesAction", "Variables/HHPFCRFFSplitVariables");
  registerSyntax("MatVecRealGradAuxKernelAction", "AuxKernels/MatVecRealGradAuxKernel");
  registerSyntax("MaterialVectorAuxKernelAction", "AuxKernels/MaterialVectorAuxKernel");
  registerSyntax("MaterialVectorGradAuxKernelAction", "AuxKernels/MaterialVectorGradAuxKernel");
  registerSyntax("MultiAuxVariablesAction", "AuxVariables/MultiAuxVariables");
  registerSyntax("PFCRFFKernelAction", "Kernels/PFCRFFKernel");
  registerSyntax("PFCRFFVariablesAction", "Variables/PFCRFFVariables");
  registerSyntax("PolycrystalColoringICAction", "ICs/PolycrystalICs/PolycrystalColoringIC");
  registerSyntax("PolycrystalElasticDrivingForceAction", "Kernels/PolycrystalElasticDrivingForce");
  registerSyntax("NonconservedAction", "Modules/PhaseField/Nonconserved/*");
  registerSyntax("PolycrystalKernelAction", "Kernels/PolycrystalKernel");
  registerSyntax("PolycrystalRandomICAction", "ICs/PolycrystalICs/PolycrystalRandomIC");
  registerSyntax("PolycrystalStoredEnergyAction", "Kernels/PolycrystalStoredEnergy");
  registerSyntax("PolycrystalVariablesAction", "Variables/PolycrystalVariables");
  registerSyntax("PolycrystalVoronoiVoidICAction", "ICs/PolycrystalICs/PolycrystalVoronoiVoidIC");
  registerSyntax("RigidBodyMultiKernelAction", "Kernels/RigidBodyMultiKernel");
  registerSyntax("Tricrystal2CircleGrainsICAction", "ICs/PolycrystalICs/Tricrystal2CircleGrainsIC");
  registerSyntax("GrandPotentialKernelAction", "Modules/PhaseField/GrandPotential");
}

void
PhaseFieldApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"PhaseFieldApp"});
  Registry::registerActionsTo(af, {"PhaseFieldApp"});
  associateSyntaxInner(s, af);
}

void
PhaseFieldApp::registerApps()
{
  registerApp(PhaseFieldApp);
}

void
PhaseFieldApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"PhaseFieldApp"});
}

void
PhaseFieldApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"PhaseFieldApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
PhaseFieldApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

extern "C" void
PhaseFieldApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  PhaseFieldApp::registerAll(f, af, s);
}
extern "C" void
PhaseFieldApp__registerApps()
{
  PhaseFieldApp::registerApps();
}
