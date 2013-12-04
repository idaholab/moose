#include "RichardsModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// UserObjects
#include "RichardsDensityConstBulk.h"
#include "RichardsDensityIdeal.h"
#include "RichardsRelPermPower.h"
#include "RichardsRelPermVG.h"
#include "RichardsRelPermVG1.h"
#include "RichardsSeffVG.h"
#include "RichardsSeffVG1.h"
#include "RichardsSat.h"

// AuxKernels
#include "RichardsSeffAux.h"
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

// Indicators
#include "RichardsFluxJumpIndicator.h"

// Postprocessors
#include "RichardsMass.h"
#include "RichardsPiecewiseLinearSinkFlux.h"
#include "NodalMaxVarChange.h"
#include "RichardsExcavFlow.h"


void
Elk::Richards::registerObjects(Factory & factory)
{
  // UserObjects
  registerUserObject(RichardsDensityConstBulk);
  registerUserObject(RichardsDensityIdeal);
  registerUserObject(RichardsRelPermPower);
  registerUserObject(RichardsRelPermVG);
  registerUserObject(RichardsRelPermVG1);
  registerUserObject(RichardsSeffVG);
  registerUserObject(RichardsSeffVG1);
  registerUserObject(RichardsSat);

  // AuxKernels
  registerAux(RichardsSeffAux);
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

  // Indicators
  registerIndicator(RichardsFluxJumpIndicator);

  // Postprocessors
  registerPostprocessor(RichardsMass);
  registerPostprocessor(RichardsPiecewiseLinearSinkFlux);
  registerPostprocessor(NodalMaxVarChange);
  registerPostprocessor(RichardsExcavFlow);

}

void
Elk::Richards::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
}
