/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RichardsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

// Actions
#include "Q2PAction.h"

// UserObjects
#include "RichardsVarNames.h"
#include "RichardsDensityConstBulk.h"
#include "RichardsDensityConstBulkCut.h"
#include "RichardsDensityIdeal.h"
#include "RichardsDensityMethane20degC.h"
#include "RichardsDensityVDW.h"
#include "RichardsRelPermMonomial.h"
#include "RichardsRelPermPower.h"
#include "RichardsRelPermVG.h"
#include "RichardsRelPermVG1.h"
#include "RichardsRelPermBW.h"
#include "RichardsRelPermPowerGas.h"
#include "Q2PRelPermPowerGas.h"
#include "RichardsSeff1VG.h"
#include "RichardsSeff1VGcut.h"
#include "RichardsSeff1BWsmall.h"
#include "RichardsSeff1RSC.h"
#include "RichardsSeff2waterVG.h"
#include "RichardsSeff2gasVG.h"
#include "RichardsSeff2waterVGshifted.h"
#include "RichardsSeff2gasVGshifted.h"
#include "RichardsSeff2waterRSC.h"
#include "RichardsSeff2gasRSC.h"
#include "RichardsSat.h"
#include "RichardsSUPGnone.h"
#include "RichardsSUPGstandard.h"
#include "RichardsSumQuantity.h"

// AuxKernels
#include "RichardsSatAux.h"
#include "RichardsSatPrimeAux.h"
#include "RichardsSeffAux.h"
#include "RichardsSeffPrimeAux.h"
#include "RichardsSeffPrimePrimeAux.h"
#include "RichardsDensityAux.h"
#include "RichardsDensityPrimeAux.h"
#include "RichardsDensityPrimePrimeAux.h"
#include "RichardsRelPermAux.h"
#include "RichardsRelPermPrimeAux.h"
#include "RichardsRelPermPrimePrimeAux.h"
#include "DarcyFluxComponent.h"

// Materials
#include "RichardsMaterial.h"
#include "PoroFullSatMaterial.h" // Used for mechanical coupling
#include "DarcyMaterial.h"
#include "Q2PMaterial.h"

// DiracKernels
#include "RichardsBorehole.h"
#include "RichardsPolyLineSink.h"
#include "Q2PBorehole.h"

// Functions
#include "RichardsExcavGeom.h"
#include "GradParsedFunction.h"
#include "Grad2ParsedFunction.h"

// Postprocessors
#include "RichardsMass.h"
#include "RichardsPiecewiseLinearSinkFlux.h"
#include "RichardsHalfGaussianSinkFlux.h"
#include "RichardsExcavFlow.h"
#include "RichardsPlotQuantity.h"
#include "Q2PPiecewiseLinearSinkFlux.h"

// Kernels
#include "RichardsMassChange.h"
#include "RichardsLumpedMassChange.h"
#include "RichardsFlux.h"
#include "RichardsFullyUpwindFlux.h"
#include "RichardsPPenalty.h"
#include "PoroFullSatTimeDerivative.h" // Used for mechanical coupling
#include "DarcyFlux.h"
#include "Q2PPorepressureFlux.h"
#include "Q2PSaturationFlux.h"
#include "Q2PSaturationDiffusion.h"
#include "Q2PNodalMass.h"
#include "Q2PNegativeNodalMassOld.h"

// BoundaryConditions
#include "RichardsExcav.h"
#include "RichardsPiecewiseLinearSink.h"
#include "RichardsHalfGaussianSink.h"
#include "Q2PPiecewiseLinearSink.h"

// Problems
#include "RichardsMultiphaseProblem.h"

template <>
InputParameters
validParams<RichardsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

RichardsApp::RichardsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  RichardsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  RichardsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags();
  RichardsApp::registerExecFlags();
}

RichardsApp::~RichardsApp() {}

void
RichardsApp::registerApps()
{
  registerApp(RichardsApp);
}

void
RichardsApp::registerObjects(Factory & factory)
{
  // UserObjects
  registerUserObject(RichardsVarNames);
  registerUserObject(RichardsDensityConstBulk);
  registerUserObject(RichardsDensityConstBulkCut);
  registerUserObject(RichardsDensityIdeal);
  registerUserObject(RichardsDensityMethane20degC);
  registerUserObject(RichardsDensityVDW);
  registerUserObject(RichardsRelPermMonomial);
  registerUserObject(RichardsRelPermPower);
  registerUserObject(RichardsRelPermVG);
  registerUserObject(RichardsRelPermVG1);
  registerUserObject(RichardsRelPermBW);
  registerUserObject(RichardsRelPermPowerGas);
  registerUserObject(Q2PRelPermPowerGas);
  registerUserObject(RichardsSeff1VG);
  registerUserObject(RichardsSeff1VGcut);
  registerUserObject(RichardsSeff1BWsmall);
  registerUserObject(RichardsSeff1RSC);
  registerUserObject(RichardsSeff2waterVG);
  registerUserObject(RichardsSeff2gasVG);
  registerUserObject(RichardsSeff2waterVGshifted);
  registerUserObject(RichardsSeff2gasVGshifted);
  registerUserObject(RichardsSeff2waterRSC);
  registerUserObject(RichardsSeff2gasRSC);
  registerUserObject(RichardsSat);
  registerUserObject(RichardsSUPGnone);
  registerUserObject(RichardsSUPGstandard);
  registerUserObject(RichardsSumQuantity);

  // AuxKernels
  registerAux(RichardsSatAux);
  registerAux(RichardsSatPrimeAux);
  registerAux(RichardsSeffAux);
  registerAux(RichardsSeffPrimeAux);
  registerAux(RichardsSeffPrimePrimeAux);
  registerAux(RichardsDensityAux);
  registerAux(RichardsDensityPrimeAux);
  registerAux(RichardsDensityPrimePrimeAux);
  registerAux(RichardsRelPermAux);
  registerAux(RichardsRelPermPrimeAux);
  registerAux(RichardsRelPermPrimePrimeAux);
  registerAux(DarcyFluxComponent);

  // Materials
  registerMaterial(RichardsMaterial);
  registerMaterial(PoroFullSatMaterial); // Used for mechanical coupling
  registerMaterial(DarcyMaterial);
  registerMaterial(Q2PMaterial);

  // DiracKernels
  registerDiracKernel(RichardsPolyLineSink);
  registerDiracKernel(RichardsBorehole);
  registerDiracKernel(Q2PBorehole);

  // Functions
  registerFunction(RichardsExcavGeom);
  registerFunction(GradParsedFunction);
  registerFunction(Grad2ParsedFunction);

  // Postprocessors
  registerPostprocessor(RichardsMass);
  registerPostprocessor(RichardsPiecewiseLinearSinkFlux);
  registerPostprocessor(RichardsHalfGaussianSinkFlux);
  registerPostprocessor(RichardsExcavFlow);
  registerPostprocessor(RichardsPlotQuantity);
  registerPostprocessor(Q2PPiecewiseLinearSinkFlux);

  // Kernels
  registerKernel(RichardsMassChange);
  registerKernel(RichardsLumpedMassChange);
  registerKernel(RichardsFlux);
  registerKernel(RichardsFullyUpwindFlux);
  registerKernel(RichardsPPenalty);
  registerKernel(PoroFullSatTimeDerivative); // Used for mechanical coupling
  registerKernel(DarcyFlux);
  registerKernel(Q2PPorepressureFlux);
  registerKernel(Q2PSaturationFlux);
  registerKernel(Q2PSaturationDiffusion);
  registerKernel(Q2PNodalMass);
  registerKernel(Q2PNegativeNodalMassOld);

  // BoundaryConditions
  registerBoundaryCondition(RichardsExcav);
  registerBoundaryCondition(RichardsPiecewiseLinearSink);
  registerBoundaryCondition(RichardsHalfGaussianSink);
  registerBoundaryCondition(Q2PPiecewiseLinearSink);

  // Problems
  registerProblem(RichardsMultiphaseProblem);
}

void
RichardsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerSyntaxTask("Q2PAction", "Q2P", "add_kernel");
  registerSyntaxTask("Q2PAction", "Q2P", "add_aux_variable");
  registerSyntaxTask("Q2PAction", "Q2P", "add_function");
  registerSyntaxTask("Q2PAction", "Q2P", "add_postprocessor");

  registerAction(Q2PAction, "add_kernel");
  registerAction(Q2PAction, "add_aux_variable");
  registerAction(Q2PAction, "add_function");
  registerAction(Q2PAction, "add_postprocessor");
}

void
RichardsApp::registerExecFlags()
{
}
