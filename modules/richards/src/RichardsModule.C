#include "RichardsModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// UserObjects
#include "RichardsDensityConstBulk.h"
#include "RichardsDensityIdeal.h"
#include "RichardsRelPermUnity.h"
#include "RichardsRelPermPower.h"
#include "RichardsRelPermVG.h"
#include "RichardsRelPermVG1.h"
#include "RichardsSeff1VG.h"
#include "RichardsSeff1VGcut.h"
#include "RichardsSeff2waterVG.h"
#include "RichardsSeff2gasVG.h"
#include "RichardsSat.h"
#include "RichardsSUPGnone.h"
#include "RichardsSUPGstandard.h"

// AuxKernels
#include "RichardsSeffAux.h"
#include "RichardsSeffPrimeAux.h"
#include "RichardsSeffPrimePrimeAux.h"
#include "RichardsDensityAux.h"
#include "RichardsDensityPrimeAux.h"
#include "RichardsDensityPrimePrimeAux.h"
#include "RichardsRelPermAux.h"
#include "RichardsMobility.h"
#include "RichardsMobilityPrime.h"
#include "RichardsMassDensity.h"
#include "RichardsMassDensityPrime.h"

// Materials
#include "RichardsMaterial.h"

// DiracKernels
#include "RichardsBorehole.h"
#include "RichardsPolyLineSink.h"

// Functions
#include "RichardsExcavGeom.h"
#include "GradParsedFunction.h"
#include "Grad2ParsedFunction.h"

// Indicators
#include "RichardsFluxJumpIndicator.h"

// Postprocessors
#include "RichardsMass.h"
#include "RichardsPiecewiseLinearSinkFlux.h"
#include "NodalMaxVarChange.h"
#include "RichardsExcavFlow.h"

// TimeSteppers
#include "FunctionControlledDT.h"

// Kernels
#include "RichardsMassChange.h"
#include "RichardsFlux.h"

  // BoundaryConditions
#include "RichardsExcav.h"
#include "RichardsPiecewiseLinearSink.h"
#include "RichardsHalfGaussianSink.h"


void
Elk::Richards::registerObjects(Factory & factory)
{
  // UserObjects
  registerUserObject(RichardsDensityConstBulk);
  registerUserObject(RichardsDensityIdeal);
  registerUserObject(RichardsRelPermUnity);
  registerUserObject(RichardsRelPermPower);
  registerUserObject(RichardsRelPermVG);
  registerUserObject(RichardsRelPermVG1);
  registerUserObject(RichardsSeff1VG);
  registerUserObject(RichardsSeff1VGcut);
  registerUserObject(RichardsSeff2waterVG);
  registerUserObject(RichardsSeff2gasVG);
  registerUserObject(RichardsSat);
  registerUserObject(RichardsSUPGnone);
  registerUserObject(RichardsSUPGstandard);

  // AuxKernels
  registerAux(RichardsSeffAux);
  registerAux(RichardsSeffPrimeAux);
  registerAux(RichardsSeffPrimePrimeAux);
  registerAux(RichardsDensityAux);
  registerAux(RichardsDensityPrimeAux);
  registerAux(RichardsDensityPrimePrimeAux);
  registerAux(RichardsRelPermAux);
  registerAux(RichardsMobility);
  registerAux(RichardsMobilityPrime);
  registerAux(RichardsMassDensity);
  registerAux(RichardsMassDensityPrime);

  // Materials
  registerMaterial(RichardsMaterial);

  // DiracKernels
  registerDiracKernel(RichardsPolyLineSink);
  registerDiracKernel(RichardsBorehole);

  // Functions
  registerFunction(RichardsExcavGeom);
  registerFunction(GradParsedFunction);
  registerFunction(Grad2ParsedFunction);

  // Indicators
  registerIndicator(RichardsFluxJumpIndicator);

  // Postprocessors
  registerPostprocessor(RichardsMass);
  registerPostprocessor(RichardsPiecewiseLinearSinkFlux);
  registerPostprocessor(NodalMaxVarChange);
  registerPostprocessor(RichardsExcavFlow);

  // TimeSteppers
  registerTimeStepper(FunctionControlledDT);

  // Kernels
  registerKernel(RichardsMassChange);
  registerKernel(RichardsFlux);

  // BoundaryConditions
  registerBoundaryCondition(RichardsExcav);
  registerBoundaryCondition(RichardsPiecewiseLinearSink);
  registerBoundaryCondition(RichardsHalfGaussianSink);

}

void
Elk::Richards::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
}
