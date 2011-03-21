#include "Moose.h"
#include "Factory.h"
#include "ImplicitSystem.h"
#include "PetscSupport.h"

// objects that can be created by MOOSE
#include "TimeDerivative.h"

#include "Steady.h"
#include "Transient.h"
#include "LooseCoupling.h"

#include "ParsedFunction.h"
#include "ParsedGradFunction.h"
#include "PiecewiseLinear.h"

// PPS
#include "AverageElementSize.h"
#include "AverageNodalVariableValue.h"
#include "ElementAverageValue.h"
#include "ElementH1Error.h"
#include "ElementH1SemiError.h"
#include "ElementIntegral.h"
#include "ElementL2Error.h"
#include "NodalVariableValue.h"
#include "PrintDOFs.h"
#include "PrintDT.h"
#include "PrintNumElems.h"
#include "PrintNumNodes.h"
#include "Reporter.h"
#include "SideFluxIntegral.h"


namespace Moose {

static bool registered = false;

void
registerObjects()
{
  if (registered)
    return;

  registerObject(TimeDerivative);

  registerObject(Steady);
  registerObject(Transient);
  registerObject(LooseCoupling);

  registerObject(ParsedFunction);
  registerObject(ParsedGradFunction);
  registerObject(PiecewiseLinear);
  // PPS
  registerObject(AverageElementSize);
  registerObject(AverageNodalVariableValue);
  registerObject(ElementAverageValue);
  registerObject(ElementH1Error);
  registerObject(ElementH1SemiError);
  registerObject(ElementIntegral);
  registerObject(ElementL2Error);
  registerObject(NodalVariableValue);
  registerObject(PrintDOFs);
  registerObject(PrintDT);
  registerObject(PrintNumElems);
  registerObject(PrintNumNodes);
  registerObject(Reporter);
  registerObject(SideFluxIntegral);

  registered = true;
}

void
setSolverDefaults(ImplicitSystem & system)
{
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetDefaults(system);
#endif //LIBMESH_HAVE_PETSC
}

} // namespace
