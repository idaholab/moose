//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "PorousFlowApp.h"
#include "Moose.h"
#include "TensorMechanicsApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "FluidPropertiesApp.h"
#include "ChemicalReactionsApp.h"

// Actions
#include "PorousFlowUnsaturated.h"
#include "PorousFlowFullySaturated.h"
#include "PorousFlowBasicTHM.h"

// UserObjects
#include "PorousFlowDictator.h"
#include "PorousFlowSumQuantity.h"
#include "PorousFlowCapillaryPressureConst.h"
#include "PorousFlowCapillaryPressureVG.h"
#include "PorousFlowCapillaryPressureRSC.h"
#include "PorousFlowCapillaryPressureBW.h"
#include "PorousFlowCapillaryPressureBC.h"
#include "PorousFlowWaterNCG.h"
#include "PorousFlowBrineCO2.h"

// DiracKernels
#include "PorousFlowSquarePulsePointSource.h"
#include "PorousFlowPeacemanBorehole.h"
#include "PorousFlowPolyLineSink.h"

// Postprocessors
#include "PorousFlowFluidMass.h"
#include "PorousFlowHeatEnergy.h"
#include "PorousFlowPlotQuantity.h"

// Materials
#include "PorousFlow1PhaseMD_Gaussian.h"
#include "PorousFlow2PhasePP.h"
#include "PorousFlow2PhasePS_VG.h"
#include "PorousFlow1PhaseP.h"
#include "PorousFlow1PhaseFullySaturated.h"
#include "PorousFlow2PhasePP_VG.h"
#include "PorousFlow2PhasePP_RSC.h"
#include "PorousFlowMassFraction.h"
#include "PorousFlow1PhaseP_VG.h"
#include "PorousFlow1PhaseP_BW.h"
#include "PorousFlow2PhasePS.h"
#include "PorousFlowBrine.h"
#include "PorousFlowEffectiveFluidPressure.h"
#include "PorousFlowPermeabilityConst.h"
#include "PorousFlowPermeabilityConstFromVar.h"
#include "PorousFlowPermeabilityKozenyCarman.h"
#include "PorousFlowPermeabilityExponential.h"
#include "PorousFlowPorosityConst.h"
#include "PorousFlowPorosityHM.h"
#include "PorousFlowPorosityHMBiotModulus.h"
#include "PorousFlowPorosityTM.h"
#include "PorousFlowPorosityTHM.h"
#include "PorousFlowRelativePermeabilityBC.h"
#include "PorousFlowRelativePermeabilityCorey.h"
#include "PorousFlowRelativePermeabilityConst.h"
#include "PorousFlowRelativePermeabilityVG.h"
#include "PorousFlowRelativePermeabilityBW.h"
#include "PorousFlowRelativePermeabilityFLAC.h"
#include "PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity.h"
#include "PorousFlowVolumetricStrain.h"
#include "PorousFlowJoiner.h"
#include "PorousFlowTemperature.h"
#include "PorousFlowThermalConductivityIdeal.h"
#include "PorousFlowThermalConductivityFromPorosity.h"
#include "PorousFlowMatrixInternalEnergy.h"
#include "PorousFlowDiffusivityConst.h"
#include "PorousFlowDiffusivityMillingtonQuirk.h"
#include "PorousFlowSingleComponentFluid.h"
#include "PorousFlowNearestQp.h"
#include "PorousFlowConstantBiotModulus.h"
#include "PorousFlowConstantThermalExpansionCoefficient.h"
#include "PorousFlowFluidStateWaterNCG.h"
#include "PorousFlowFluidStateBrineCO2.h"

// Kernels
#include "PorousFlowAdvectiveFlux.h"
#include "PorousFlowMassTimeDerivative.h"
#include "PorousFlowEffectiveStressCoupling.h"
#include "PorousFlowMassVolumetricExpansion.h"
#include "PorousFlowEnergyTimeDerivative.h"
#include "PorousFlowHeatConduction.h"
#include "PorousFlowHeatAdvection.h"
#include "PorousFlowDispersiveFlux.h"
#include "PorousFlowHeatVolumetricExpansion.h"
#include "PorousFlowPlasticHeatEnergy.h"
#include "PorousFlowDesorpedMassTimeDerivative.h"
#include "PorousFlowDesorpedMassVolumetricExpansion.h"
#include "PorousFlowMassRadioactiveDecay.h"
#include "PorousFlowFullySaturatedDarcyBase.h"
#include "PorousFlowFullySaturatedDarcyFlow.h"
#include "PorousFlowFullySaturatedHeatAdvection.h"
#include "PorousFlowFullySaturatedMassTimeDerivative.h"
#include "PorousFlowExponentialDecay.h"

// BoundaryConditions
#include "PorousFlowSink.h"
#include "PorousFlowPiecewiseLinearSink.h"
#include "PorousFlowHalfGaussianSink.h"
#include "PorousFlowHalfCubicSink.h"

// AuxKernels
#include "PorousFlowDarcyVelocityComponent.h"
#include "PorousFlowDarcyVelocityComponentLowerDimensional.h"
#include "PorousFlowPropertyAux.h"

// Functions
#include "MovingPlanarFront.h"

// ICs
#include "PorousFlowFluidStateWaterNCGIC.h"
#include "PorousFlowFluidStateBrineCO2IC.h"

template <>
InputParameters
validParams<PorousFlowApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

PorousFlowApp::PorousFlowApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  PorousFlowApp::registerObjectDepends(_factory);
  PorousFlowApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PorousFlowApp::associateSyntaxDepends(_syntax, _action_factory);
  PorousFlowApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  PorousFlowApp::registerExecFlags(_factory);
}

PorousFlowApp::~PorousFlowApp() {}

// External entry point for dynamic application loading
extern "C" void
PorousFlowApp__registerApps()
{
  PorousFlowApp::registerApps();
}
void
PorousFlowApp::registerApps()
{
  registerApp(PorousFlowApp);
}

void
PorousFlowApp::registerObjectDepends(Factory & factory)
{
  TensorMechanicsApp::registerObjects(factory);
  FluidPropertiesApp::registerObjects(factory);
  ChemicalReactionsApp::registerObjects(factory);
}

// External entry point for dynamic object registration
extern "C" void
PorousFlowApp__registerObjects(Factory & factory)
{
  PorousFlowApp::registerObjects(factory);
}
void
PorousFlowApp::registerObjects(Factory & factory)
{
  // UserObjects
  registerUserObject(PorousFlowDictator);
  registerUserObject(PorousFlowSumQuantity);
  registerUserObject(PorousFlowCapillaryPressureConst);
  registerUserObject(PorousFlowCapillaryPressureVG);
  registerUserObject(PorousFlowCapillaryPressureRSC);
  registerUserObject(PorousFlowCapillaryPressureBW);
  registerUserObject(PorousFlowCapillaryPressureBC);
  registerUserObject(PorousFlowWaterNCG);
  registerUserObject(PorousFlowBrineCO2);

  // DiracKernels
  registerDiracKernel(PorousFlowSquarePulsePointSource);
  registerDiracKernel(PorousFlowPeacemanBorehole);
  registerDiracKernel(PorousFlowPolyLineSink);

  // Postprocessors
  registerPostprocessor(PorousFlowFluidMass);
  registerPostprocessor(PorousFlowHeatEnergy);
  registerPostprocessor(PorousFlowPlotQuantity);

  // Materials
  registerMaterial(PorousFlow1PhaseMD_Gaussian);
  registerMaterial(PorousFlow2PhasePP);
  registerMaterial(PorousFlow2PhasePS_VG);
  registerMaterial(PorousFlow1PhaseP);
  registerMaterial(PorousFlow1PhaseFullySaturated);
  registerMaterial(PorousFlow2PhasePP_VG);
  registerMaterial(PorousFlow2PhasePP_RSC);
  registerMaterial(PorousFlowMassFraction);
  registerMaterial(PorousFlow1PhaseP_VG);
  registerMaterial(PorousFlow1PhaseP_BW);
  registerMaterial(PorousFlow2PhasePS);
  registerMaterial(PorousFlowBrine);
  registerMaterial(PorousFlowEffectiveFluidPressure);
  registerMaterial(PorousFlowPermeabilityConst);
  registerMaterial(PorousFlowPermeabilityConstFromVar);
  registerMaterial(PorousFlowPermeabilityKozenyCarman);
  registerMaterial(PorousFlowPermeabilityExponential);
  registerMaterial(PorousFlowPorosityConst);
  registerMaterial(PorousFlowPorosityHM);
  registerMaterial(PorousFlowPorosityHMBiotModulus);
  registerMaterial(PorousFlowPorosityTM);
  registerMaterial(PorousFlowPorosityTHM);
  registerMaterial(PorousFlowRelativePermeabilityBC);
  registerMaterial(PorousFlowRelativePermeabilityCorey);
  registerMaterial(PorousFlowRelativePermeabilityConst);
  registerMaterial(PorousFlowRelativePermeabilityVG);
  registerMaterial(PorousFlowRelativePermeabilityBW);
  registerMaterial(PorousFlowRelativePermeabilityFLAC);
  registerMaterial(PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity);
  registerMaterial(PorousFlowVolumetricStrain);
  registerMaterial(PorousFlowJoiner);
  registerMaterial(PorousFlowTemperature);
  registerMaterial(PorousFlowThermalConductivityIdeal);
  registerMaterial(PorousFlowThermalConductivityFromPorosity);
  registerMaterial(PorousFlowMatrixInternalEnergy);
  registerMaterial(PorousFlowDiffusivityConst);
  registerMaterial(PorousFlowDiffusivityMillingtonQuirk);
  registerMaterial(PorousFlowSingleComponentFluid);
  registerMaterial(PorousFlowNearestQp);
  registerMaterial(PorousFlowConstantBiotModulus);
  registerMaterial(PorousFlowConstantThermalExpansionCoefficient);
  registerMaterial(PorousFlowFluidStateWaterNCG);
  registerMaterial(PorousFlowFluidStateBrineCO2);

  // Kernels
  registerKernel(PorousFlowAdvectiveFlux);
  registerKernel(PorousFlowMassTimeDerivative);
  registerKernel(PorousFlowEffectiveStressCoupling);
  registerKernel(PorousFlowMassVolumetricExpansion);
  registerKernel(PorousFlowEnergyTimeDerivative);
  registerKernel(PorousFlowHeatConduction);
  registerKernel(PorousFlowHeatAdvection);
  registerKernel(PorousFlowDispersiveFlux);
  registerKernel(PorousFlowHeatVolumetricExpansion);
  registerKernel(PorousFlowPlasticHeatEnergy);
  registerKernel(PorousFlowDesorpedMassTimeDerivative);
  registerKernel(PorousFlowDesorpedMassVolumetricExpansion);
  registerKernel(PorousFlowMassRadioactiveDecay);
  registerKernel(PorousFlowFullySaturatedDarcyBase);
  registerKernel(PorousFlowFullySaturatedDarcyFlow);
  registerKernel(PorousFlowFullySaturatedHeatAdvection);
  registerKernel(PorousFlowFullySaturatedMassTimeDerivative);
  registerKernel(PorousFlowExponentialDecay);

  // BoundaryConditions
  registerBoundaryCondition(PorousFlowSink);
  registerBoundaryCondition(PorousFlowPiecewiseLinearSink);
  registerBoundaryCondition(PorousFlowHalfGaussianSink);
  registerBoundaryCondition(PorousFlowHalfCubicSink);

  // AuxKernels
  registerAuxKernel(PorousFlowDarcyVelocityComponent);
  registerAuxKernel(PorousFlowDarcyVelocityComponentLowerDimensional);
  registerAuxKernel(PorousFlowPropertyAux);

  // Functions
  registerFunction(MovingPlanarFront);

  // ICs
  registerInitialCondition(PorousFlowFluidStateWaterNCGIC);
  registerInitialCondition(PorousFlowFluidStateBrineCO2IC);
}

void
PorousFlowApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
}

// External entry point for dynamic syntax association
extern "C" void
PorousFlowApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  PorousFlowApp::associateSyntax(syntax, action_factory);
}
void
PorousFlowApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  syntax.registerActionSyntax("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_user_object");
  syntax.registerActionSyntax("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_kernel");
  syntax.registerActionSyntax("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_material");
  syntax.registerActionSyntax("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_aux_variable");
  syntax.registerActionSyntax("PorousFlowUnsaturated", "PorousFlowUnsaturated", "add_aux_kernel");
  registerAction(PorousFlowUnsaturated, "add_user_object");
  registerAction(PorousFlowUnsaturated, "add_kernel");
  registerAction(PorousFlowUnsaturated, "add_material");
  registerAction(PorousFlowUnsaturated, "add_aux_variable");
  registerAction(PorousFlowUnsaturated, "add_aux_kernel");

  syntax.registerActionSyntax(
      "PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_user_object");
  syntax.registerActionSyntax("PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_kernel");
  syntax.registerActionSyntax(
      "PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_material");
  syntax.registerActionSyntax(
      "PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_aux_variable");
  syntax.registerActionSyntax(
      "PorousFlowFullySaturated", "PorousFlowFullySaturated", "add_aux_kernel");
  registerAction(PorousFlowFullySaturated, "add_user_object");
  registerAction(PorousFlowFullySaturated, "add_kernel");
  registerAction(PorousFlowFullySaturated, "add_material");
  registerAction(PorousFlowFullySaturated, "add_aux_variable");
  registerAction(PorousFlowFullySaturated, "add_aux_kernel");

  syntax.registerActionSyntax("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_user_object");
  syntax.registerActionSyntax("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_kernel");
  syntax.registerActionSyntax("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_material");
  syntax.registerActionSyntax("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_aux_variable");
  syntax.registerActionSyntax("PorousFlowBasicTHM", "PorousFlowBasicTHM", "add_aux_kernel");
  registerAction(PorousFlowBasicTHM, "add_user_object");
  registerAction(PorousFlowBasicTHM, "add_kernel");
  registerAction(PorousFlowBasicTHM, "add_material");
  registerAction(PorousFlowBasicTHM, "add_aux_variable");
  registerAction(PorousFlowBasicTHM, "add_aux_kernel");
}

// External entry point for dynamic execute flag registration
extern "C" void
PorousFlowApp__registerExecFlags(Factory & factory)
{
  PorousFlowApp::registerExecFlags(factory);
}
void
PorousFlowApp::registerExecFlags(Factory & /*factory*/)
{
}
