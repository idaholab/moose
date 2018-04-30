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

template <>
InputParameters
validParams<PhaseFieldApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("PhaseFieldApp");

PhaseFieldApp::PhaseFieldApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  PhaseFieldApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PhaseFieldApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  PhaseFieldApp::registerExecFlags(_factory);
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
  Registry::registerObjectsTo(factory, {"PhaseFieldApp"});
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
  Registry::registerActionsTo(action_factory, {"PhaseFieldApp"});

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
  registerSyntax("MortarPeriodicAction", "Modules/PhaseField/MortarPeriodicity/*");
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

// External entry point for dynamic execute flag registration
extern "C" void
PhaseFieldApp__registerExecFlags(Factory & factory)
{
  PhaseFieldApp::registerExecFlags(factory);
}
void
PhaseFieldApp::registerExecFlags(Factory & /*factory*/)
{
}
