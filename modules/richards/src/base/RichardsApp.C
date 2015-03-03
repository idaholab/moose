/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "RichardsApp.h"
#include "Moose.h"
#include "AppFactory.h"

// UserObjects
#include "RichardsVarNames.h"
#include "RichardsDensityConstBulk.h"
#include "RichardsDensityIdeal.h"
#include "RichardsDensityMethane20degC.h"
#include "RichardsDensityVDW.h"
#include "RichardsRelPermMonomial.h"
#include "RichardsRelPermPower.h"
#include "RichardsRelPermVG.h"
#include "RichardsRelPermVG1.h"
#include "RichardsRelPermBW.h"
#include "RichardsRelPermPowerGas.h"
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
#include "FunctionOfVariableAux.h"

// Materials
#include "RichardsMaterial.h"

// DiracKernels
#include "RichardsBorehole.h"
#include "RichardsPolyLineSink.h"

// Functions
#include "RichardsExcavGeom.h"
#include "GradParsedFunction.h"
#include "Grad2ParsedFunction.h"

// Postprocessors
#include "RichardsMass.h"
#include "RichardsPiecewiseLinearSinkFlux.h"
#include "RichardsHalfGaussianSinkFlux.h"
#include "NodalMaxVarChange.h"
#include "RichardsExcavFlow.h"
#include "RichardsPlotQuantity.h"

// Kernels
#include "RichardsMassChange.h"
#include "RichardsLumpedMassChange.h"
#include "RichardsFlux.h"
#include "RichardsFullyUpwindFlux.h"
#include "RichardsPPenalty.h"

  // BoundaryConditions
#include "RichardsExcav.h"
#include "RichardsPiecewiseLinearSink.h"
#include "RichardsHalfGaussianSink.h"

// Problems
#include "RichardsMultiphaseProblem.h"

template<>
InputParameters validParams<RichardsApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = true;
  params.set<bool>("use_legacy_uo_aux_computation") = false;

  return params;
}

RichardsApp::RichardsApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  RichardsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  RichardsApp::associateSyntax(_syntax, _action_factory);
}

RichardsApp::~RichardsApp()
{
}

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
  registerUserObject(RichardsDensityIdeal);
  registerUserObject(RichardsDensityMethane20degC);
  registerUserObject(RichardsDensityVDW);
  registerUserObject(RichardsRelPermMonomial);
  registerUserObject(RichardsRelPermPower);
  registerUserObject(RichardsRelPermVG);
  registerUserObject(RichardsRelPermVG1);
  registerUserObject(RichardsRelPermBW);
  registerUserObject(RichardsRelPermPowerGas);
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
  registerAux(FunctionOfVariableAux);

  // Materials
  registerMaterial(RichardsMaterial);

  // DiracKernels
  registerDiracKernel(RichardsPolyLineSink);
  registerDiracKernel(RichardsBorehole);

  // Functions
  registerFunction(RichardsExcavGeom);
  registerFunction(GradParsedFunction);
  registerFunction(Grad2ParsedFunction);

  // Postprocessors
  registerPostprocessor(RichardsMass);
  registerPostprocessor(RichardsPiecewiseLinearSinkFlux);
  registerPostprocessor(RichardsHalfGaussianSinkFlux);
  registerPostprocessor(NodalMaxVarChange);
  registerPostprocessor(RichardsExcavFlow);
  registerPostprocessor(RichardsPlotQuantity);

  // Kernels
  registerKernel(RichardsMassChange);
  registerKernel(RichardsLumpedMassChange);
  registerKernel(RichardsFlux);
  registerKernel(RichardsFullyUpwindFlux);
  registerKernel(RichardsPPenalty);

  // BoundaryConditions
  registerBoundaryCondition(RichardsExcav);
  registerBoundaryCondition(RichardsPiecewiseLinearSink);
  registerBoundaryCondition(RichardsHalfGaussianSink);

  // Problems
  registerProblem(RichardsMultiphaseProblem);
}

void
RichardsApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
